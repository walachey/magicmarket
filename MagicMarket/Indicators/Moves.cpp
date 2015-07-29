#include "Moves.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{

		Moves::Moves(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
			plusDMMA  = std::numeric_limits<double>::quiet_NaN();
			minusDMMA = std::numeric_limits<double>::quiet_NaN();
			upMA   = std::numeric_limits<double>::quiet_NaN();
			downMA = std::numeric_limits<double>::quiet_NaN();
		}

		Moves::~Moves()
		{
		}

		void Moves::declareExports() const
		{
			exportVariable("pDMMA", getPlusDMMA, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("mDMMA", getMinusDMMA, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));

			exportVariable("upMA", getUpMA, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("downMA", getDownMA, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void Moves::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;

			MM::TimePeriod now = stock->getTimePeriod(time - seconds, time);
			const MM::PossibleDecimal high = now.getHigh();
			const MM::PossibleDecimal low = now.getLow();
			const MM::PossibleDecimal close = now.getClose();

			MM::TimePeriod then = stock->getTimePeriod(time - 2 * seconds, time - seconds);
			const MM::PossibleDecimal oldHigh = then.getHigh();
			const MM::PossibleDecimal oldLow = then.getLow();
			const MM::PossibleDecimal oldClose = then.getClose();

			if (!high.get() || !low.get() || !oldHigh.get() || !oldLow.get()) return;

			/* Up and Down changes for the ADX */
			const double upMove   = *high - *oldHigh;
			const double downMove = *low  - *oldLow;

			double plusDM = 0.0, minusDM = 0.0;
			if (upMove > downMove && upMove > 0.0) plusDM = upMove;
			else 
			if (downMove > upMove && downMove > 0.0) minusDM = downMove;

			const double historyDouble = static_cast<double>(history);

			plusDMMA = Math::MA(plusDMMA, plusDM, history);
			minusDMMA = Math::MA(minusDMMA, minusDM, history);

			/* Simple Up and Down moves for the RSI */
			if (!close.get() || !oldClose.get()) return;
			const double move = *close - *oldClose;
			double U(0.0), D(0.0);
			if (move > 0.0) U = move;
			else if (move < 0.0) D = move;

			upMA   = Math::MA(upMA, U, history);
			downMA = Math::MA(downMA, D, history);
		}
	};
};
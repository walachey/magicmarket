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
		}

		Moves::~Moves()
		{
		}

		void Moves::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;

			MM::TimePeriod now = stock->getTimePeriod(time - seconds, time);
			const MM::PossibleDecimal high = now.getHigh();
			const MM::PossibleDecimal low = now.getLow();

			MM::TimePeriod then = stock->getTimePeriod(time - 2 * seconds, time - seconds);
			const MM::PossibleDecimal oldHigh = then.getHigh();
			const MM::PossibleDecimal oldLow = then.getLow();

			if (!high.get() || !low.get() || !oldHigh.get() || !oldLow.get()) return;

			const double upMove   = *high.get() - *oldHigh.get();
			const double downMove = *low.get()  - *oldLow.get();

			double plusDM = 0.0, minusDM = 0.0;
			if (upMove > downMove && upMove > 0.0) plusDM = upMove;
			else 
			if (downMove > upMove && downMove > 0.0) minusDM = downMove;

			const double historyDouble = static_cast<double>(history);

			if (std::isnan(plusDMMA))
				plusDMMA = plusDM;
			else
				plusDMMA = ((historyDouble - 1.0) * plusDMMA + plusDM) / historyDouble;

			if (std::isnan(minusDMMA))
				minusDMMA = minusDM;
			else
				minusDMMA = ((historyDouble - 1.0) * minusDMMA + minusDM) / historyDouble;
		}
	};
};
#include "Moves.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{
		void Moves::reset()
		{
			plusDMMA = plusDMMA_pushed = std::numeric_limits<double>::quiet_NaN();
			minusDMMA = minusDMMA_pushed = std::numeric_limits<double>::quiet_NaN();
			upMA = upMA_pushed = std::numeric_limits<double>::quiet_NaN();
			downMA = downMA_pushed = std::numeric_limits<double>::quiet_NaN();

			momentumMA = momentumMA_pushed = std::numeric_limits<double>::quiet_NaN();
			momentumAbsMA = momentumAbsMA_pushed = std::numeric_limits<double>::quiet_NaN();

			lastMAPush = 0;
		}

		Moves::Moves(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
		
		}

		Moves::~Moves()
		{
		}

		void Moves::declareExports() const
		{
			exportVariable("pDMMA", std::bind(&Moves::getPlusDMMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("mDMMA", std::bind(&Moves::getMinusDMMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));

			exportVariable("upMA", std::bind(&Moves::getUpMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("downMA", std::bind(&Moves::getDownMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));

			exportVariable("momentumMA", std::bind(&Moves::getMomentumMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("momentumMAAbs", std::bind(&Moves::getAbsoluteMomentumMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
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

			bool refreshMovingAverages = false;
			if (time >= (lastMAPush + seconds))
			{
				refreshMovingAverages = true;
				lastMAPush = time;
			}

			/* Up and Down changes for the ADX */
			const double upMove   = *high - *oldHigh;
			const double downMove = *low  - *oldLow;

			double plusDM = 0.0, minusDM = 0.0;
			if (upMove > downMove && upMove > 0.0) plusDM = upMove;
			else 
			if (downMove > upMove && downMove > 0.0) minusDM = downMove;

			const double historyDouble = static_cast<double>(history);

			plusDMMA = Math::MA(plusDMMA_pushed, plusDM, history);
			minusDMMA = Math::MA(minusDMMA_pushed, minusDM, history);

			if (refreshMovingAverages)
			{
				plusDMMA_pushed = plusDMMA;
				minusDMMA_pushed = minusDMMA;
			}

			assert(std::isnormal(plusDMMA) || plusDMMA == 0.0);
			assert(std::isnormal(minusDMMA) || minusDMMA == 0.0);
			assert(plusDMMA >= -2.0 && plusDMMA <= +2.0);
			assert(minusDMMA >= -2.0 && minusDMMA <= +2.0);
			/* Simple Up and Down moves for the RSI */
			if (!close.get() || !oldClose.get()) return;
			const double move = *close - *oldClose;
			double U(0.0), D(0.0);
			if (move > 0.0) U = move;
			else if (move < 0.0) D = -move;

			upMA   = Math::MA(upMA_pushed, U, history);
			downMA = Math::MA(downMA_pushed, D, history);

			if (refreshMovingAverages)
			{
				upMA_pushed = upMA;
				downMA_pushed = downMA;
			}

			/* smoothed momentum for TSI */
			momentumMA    = Math::MA2(momentumMA_pushed, move, history);
			momentumAbsMA = Math::MA2(momentumAbsMA_pushed, std::abs(move), history);

			if (refreshMovingAverages)
			{
				momentumMA_pushed = momentumMA;
				momentumAbsMA_pushed = momentumAbsMA;
			}
		}
	};
};
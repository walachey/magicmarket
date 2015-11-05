#include "StochasticOscillator.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{
		void StochasticOscillator::reset()
		{
			percentK = std::numeric_limits<double>::quiet_NaN();
			percentD = std::numeric_limits<double>::quiet_NaN();
		}

		StochasticOscillator::StochasticOscillator(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{

		}

		StochasticOscillator::~StochasticOscillator()
		{
		}

		void StochasticOscillator::declareExports() const
		{
			exportVariable("%K", std::bind(&StochasticOscillator::getPercentK, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("%D", std::bind(&StochasticOscillator::getPercentD, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void StochasticOscillator::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;
			MM::TimePeriod now = stock->getTimePeriod(time - history * seconds, time);
			const PossibleDecimal close = now.getClose();
			const PossibleDecimal high  = now.getHigh();
			const PossibleDecimal low   = now.getLow();
			if (!close.get() || !high.get() || !low.get()) return;

			percentK = 100.0 * ((*close - *low) / (*high - *low));
			percentD = Math::MA(percentD, percentK, 3);
		}
	};
};
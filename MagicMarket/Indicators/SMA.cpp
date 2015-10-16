#include "SMA.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{

		void SMA::reset()
		{
			sma = std::numeric_limits<double>::quiet_NaN();
			sma2 = std::numeric_limits<double>::quiet_NaN();
			sma2abs = std::numeric_limits<double>::quiet_NaN();
		}

		SMA::SMA(std::string currencyPair, int history, int seconds, std::function<double()> valueProvider) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds),
			valueProvider(valueProvider)
		{
		}


		SMA::~SMA()
		{
		}

		void SMA::declareExports() const
		{
			assert(seconds > 0 || !customDescription.empty());
			exportVariable("SMA", std::bind(&SMA::getSMA, this), customDescription + "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("SMA2", std::bind(&SMA::getSMA2, this), customDescription + "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("SMA2Abs", std::bind(&SMA::getSMA2Abs, this), customDescription + "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void SMA::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			double value;
			if (valueProvider == nullptr)
			{
				Stock *stock = market.getStock(currencyPair);
				if (stock == nullptr) return;
				MM::TimePeriod now = stock->getTimePeriod(time - seconds, time);
				const PossibleDecimal price = now.getClose();
				if (!price.get()) return;
				value = *price;
			}
			else
				value = valueProvider();

			if (std::isnan(value)) return;

			sma     = Math::MA (sma, value, history);
			sma2    = Math::MA2(sma2, value, history);
			sma2abs = Math::MA2(sma2abs, std::abs(value), history);
		}
	};
};
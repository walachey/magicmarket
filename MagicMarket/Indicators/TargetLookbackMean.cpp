#include "TargetLookbackMean.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"
#include "Statistics.h"

#include "Helpers.h"

#include <algorithm>

namespace MM
{
	namespace Indicators
	{
		void TargetLookbackMean::reset()
		{
			onlineMean.reset();
			currentMean = std::numeric_limits<double>::quiet_NaN();
			lastPushed = 0;
		}

		TargetLookbackMean::TargetLookbackMean(std::string currencyPair, int minutesLookback) :
			currencyPair(currencyPair),
			minutesLookback(minutesLookback)
		{
			
		}

		TargetLookbackMean::~TargetLookbackMean()
		{
		}

		void TargetLookbackMean::declareExports() const
		{
			exportVariable("TargetMean", std::bind(&TargetLookbackMean::getTargetMean, this), "lookback " + std::to_string(minutesLookback) + ", currency " + currencyPair);
		}

		double TargetLookbackMean::calculateTarget(TimePeriod &period)
		{
			const std::vector<QuantLib::Decimal> price = period.toVector(ONEMINUTE);

			QuantLib::Decimal min = 0.0;
			QuantLib::Decimal max = 0.0;
			int lastSign = 0;
			for (size_t i = 1; i < price.size(); ++i)
			{
				QuantLib::Decimal currentChange = price[i] - price[0];

				int sign = Math::signum(currentChange);

				if (lastSign != 0 && sign != lastSign) break;
				lastSign = sign;

				if (currentChange > max) max = currentChange;
				if (currentChange < min) min = currentChange;
			}
			assert(min == 0.0 || max == 0.0);
			decltype(min) value = 0.0;

			if (std::abs(min) > std::abs(max)) value = min;
			else value = max;
			return value / ONEPIP;
		};

		void TargetLookbackMean::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;
			TimePeriod period = stock->getTimePeriod(time - 60 * minutesLookback, time);

			PossibleDecimal open;
			open = period.getOpen();
			if (!open) return;

			double target = calculateTarget(period);

			if (time > lastPushed + ONEMINUTE)
			{
				onlineMean.update(target);
				currentMean = onlineMean.get();
			}
			else
			{
				currentMean = onlineMean.preview(target);
			}
		}
	};
};
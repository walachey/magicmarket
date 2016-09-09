#include "LocalRelativeChange.h"
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
		void LocalRelativeChange::reset()
		{
			const double *lastValue = &lookbackDerivatives.back();
			lookbackDerivatives.clear();
			for (size_t i = 0; i < lookbackDurations.size(); ++i)
				lookbackDerivatives.push_back(std::numeric_limits<double>::quiet_NaN());
			// The memory location should not change.
			assert(lastValue == &lookbackDerivatives.back());
		}

		LocalRelativeChange::LocalRelativeChange(std::string currencyPair, std::vector<int> lookbackDurations) :
			currencyPair(currencyPair),
			lookbackDurations(lookbackDurations)
		{
			lookbackDerivatives.resize(lookbackDurations.size());
		}

		LocalRelativeChange::~LocalRelativeChange()
		{
		}

		void LocalRelativeChange::declareExports() const
		{
			std::string allLookbacks = "{";
			for (size_t i = 0; i < lookbackDurations.size(); ++i)
				allLookbacks += ((i == 0) ? "" : ", ") + std::to_string(lookbackDurations[i]);
			allLookbacks += "}";

			// Do not add the maximum/minimum lookback as an observation, because it is 0 all the time.
			const int maxLookbackDuration = *std::max_element(lookbackDurations.begin(), lookbackDurations.end());
			const int minLookbackDuration = *std::min_element(lookbackDurations.begin(), lookbackDurations.end());

			const std::string namePrefix = currencyPair + "_local_";
			size_t index = 0;
			for (int const &lookback : lookbackDurations)
			{
				if ((lookback != maxLookbackDuration) && (lookback != minLookbackDuration))
				{
					const std::string name = namePrefix + std::to_string(lookback) + "s";
					const std::string desc = "Local relative change of " + currencyPair + " with lookback " + std::to_string(lookback) + " " + allLookbacks;
					statistics.addVariable(Variable(name, &lookbackDerivatives[index], desc));
				}
				index += 1;
			}
		}

		void LocalRelativeChange::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;

			int maxLookbackIndex = -1;
			int minLookbackIndex = -1;
			bool pricesMissing = false;

			for (size_t i = 0; i < lookbackDurations.size(); ++i)
			{
				const int &lookback = lookbackDurations[i];

				// Figure out first value of series as the basis for the relative data.
				if (maxLookbackIndex == -1 || lookback > lookbackDurations[maxLookbackIndex])
					maxLookbackIndex = i;
				if (minLookbackIndex == -1 || lookback < lookbackDurations[minLookbackIndex])
					minLookbackIndex = i;

				double &value = lookbackDerivatives[i];

				MM::TimePeriod now = stock->getTimePeriod(time - lookback);
				const PossibleDecimal price = now.getOpen();
				if (!price.get())
				{
					value = std::numeric_limits<double>::quiet_NaN();
					pricesMissing = true;
					continue;
				}
				else value = *price;
			}
			assert(minLookbackIndex != -1);
			assert(maxLookbackIndex != -1);
			//assert(std::isnan(lookbackDerivatives[maxLookbackIndex]) || !std::isnan(lookbackDerivatives[minLookbackIndex]));

			if (pricesMissing)
			{
				for (auto & value : lookbackDerivatives)
					value = std::numeric_limits<double>::quiet_NaN();
				return;
			}
			// And normalize the data.

			// Figure out normalization factor to use.
			double estimatedStdDeviation = 1.0;
			MM::TimePeriod normalizationPeriod = stock->getTimePeriod(time - 2 * ONEHOUR);
			std::vector<double> stdDevEstimationSamples = normalizationPeriod.toVector(10 * ONEMINUTE);
			if (stdDevEstimationSamples.size() > 5)
			{
				stdDevEstimationSamples = Math::derive(stdDevEstimationSamples);
				assert(stdDevEstimationSamples.size() > 2);
				estimatedStdDeviation = Math::stddev(stdDevEstimationSamples);
			}

			// First, detrend it (assuming a linear trend during the lookback timeframe).
			const double trendLength = static_cast<double>(lookbackDurations[maxLookbackIndex] - lookbackDurations[minLookbackIndex]);
			assert(trendLength > 0.0);
			const double trendAbsolute = lookbackDerivatives[minLookbackIndex] - lookbackDerivatives[maxLookbackIndex];
			const double trendGradient = trendAbsolute / trendLength;
			assert(!std::isnan(trendGradient));

			const double firstDataValue = (maxLookbackIndex == -1) ? std::numeric_limits<double>::quiet_NaN() : lookbackDerivatives[maxLookbackIndex];
			// Center it first and remove linear trend.
			for (size_t i = 0; i < lookbackDerivatives.size(); ++i)
			{
				double & value = lookbackDerivatives[i];
				const int lookback = lookbackDurations[i];
				const double offset = static_cast<double>(lookbackDurations[maxLookbackIndex] - lookback);
				// Make it relative to the first observation.
				value = (value - firstDataValue);
				// Remove the constant linear trend.
				value = (value - offset * trendGradient);
				assert(std::isnormal(value) || value == 0.0);
			}
			assert(!std::isnan(estimatedStdDeviation));
			// And then normalize it by estimated stddev.
			for (double & value : lookbackDerivatives)
				value /= estimatedStdDeviation;
			// Due to de-trending, we have some assertions.
			assert(std::isnan(lookbackDerivatives[maxLookbackIndex]) || std::abs(lookbackDerivatives[maxLookbackIndex]) < 1.e-10);
			assert(std::isnan(lookbackDerivatives[minLookbackIndex]) || std::abs(lookbackDerivatives[minLookbackIndex]) < 1.e-10);
		}
	};
};
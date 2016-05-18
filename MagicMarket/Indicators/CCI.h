#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class Moves;

		// https://en.wikipedia.org/wiki/Commodity_channel_index
		class CCI : public Base
		{
		public:
			virtual void reset() override;
			CCI(std::string currencyPair, int history, int seconds);
			virtual ~CCI();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const CCI* other = dynamic_cast<const CCI*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getCCI() const { return cci; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double cci;

			double typicalPriceMA;
			std::time_t lastUpdateTime;

			// For the online mean absolute deviation.
			struct OnlineMeanAbsoluteDeviation
			{
				int n = 0;
				double mean = 0.0;
				double M2 = 0.0;
				double MAD = 0.0;

				void reset();
				void update(double newValue);

			} OnlineMAD;
		};

	};
};
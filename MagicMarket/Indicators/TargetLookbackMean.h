#pragma once

#include "Base.h"
#include <string>

#include <Helpers.h>

namespace MM
{
	class Stock;
	class TimePeriod;

	namespace Indicators
	{
		class TargetLookbackMean : public Base
		{
		public:
			virtual void reset() override;
			TargetLookbackMean(std::string currencyPair, int minutesLookback);
			virtual ~TargetLookbackMean();

			virtual void declareExports() const override;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const TargetLookbackMean* other = dynamic_cast<const TargetLookbackMean*>(&otherBase);
				if (other == nullptr) return false;
				return (other->currencyPair == currencyPair) && other->minutesLookback == minutesLookback;
			}
			static double TargetLookbackMean::calculateTarget(TimePeriod &period);
			double getTargetMean() const { return currentMean; }
		private:
			std::string currencyPair;
			int minutesLookback;
			::MM::Math::OnlineMean onlineMean;
			double currentMean;
			std::time_t lastPushed;
		};

	};
};
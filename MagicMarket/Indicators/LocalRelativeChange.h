#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class LocalRelativeChange : public Base
		{
		public:
			virtual void reset() override;
			LocalRelativeChange(std::string currencyPair, std::vector<int> lookbackDurations);
			virtual ~LocalRelativeChange();

			virtual void declareExports() const override;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const LocalRelativeChange* other = dynamic_cast<const LocalRelativeChange*>(&otherBase);
				if (other == nullptr) return false;
				return (other->currencyPair == currencyPair) && other->lookbackDurations == lookbackDurations;
			}

		private:
			std::string currencyPair;
			std::vector<int> lookbackDurations;
			std::vector<double> lookbackDerivatives;
		};

	};
};
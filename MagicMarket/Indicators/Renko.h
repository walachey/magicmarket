#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		// http://stockcharts.com/school/doku.php?id=chart_school:chart_analysis:renko
		class Renko : public Base
		{
		public:
			virtual void reset() override;
			Renko(std::string currencyPair, int history, double minimumChange);
			virtual ~Renko();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const Renko* other = dynamic_cast<const Renko*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->minimumChange == minimumChange) && (other->currencyPair == currencyPair);
			}

			std::vector<double> getBars(int n) const;
			double getCurrentDirection() const;

			int getOffsetIndex(int offset) const;

		private:
			std::string currencyPair;
			int history;
			double minimumChange;

			int currentBarIndex;
			std::vector<double> bars;
			double lastBarAt;
		};

	};
};
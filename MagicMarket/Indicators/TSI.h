#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class Moves;

		// https://en.wikipedia.org/wiki/Relative_strength_index
		class TSI : public Base
		{
		public:
			TSI(std::string currencyPair, int history, int seconds);
			virtual ~TSI() {}

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const TSI* other = dynamic_cast<const TSI*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getTSI() { return tsi; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double tsi;
			double momentumDoubleMA;
			double absMomentumDoubleMA;
			Moves *moves;
		};

	};
};
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
		class RSI : public Base
		{
		public:
			RSI(std::string currencyPair, int history, int seconds);
			virtual ~RSI();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const RSI* other = dynamic_cast<const RSI*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getRSI() { return rsi; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double rsi;

			Moves *moves;
		};

	};
};
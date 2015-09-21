#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		// https://en.wikipedia.org/wiki/Average_true_range
		class ATR : public Base
		{
		public:
			void reset() override;
			ATR(std::string currencyPair, int history, int seconds);
			virtual ~ATR();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const ATR* other = dynamic_cast<const ATR*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			static double getTrueRange(MM::Stock *stock, const std::time_t &time, const int &duration);

			double getATRMA() const { return value; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double value;
		};

	};
};
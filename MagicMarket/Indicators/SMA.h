#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class SMA : public Base
		{
		public:
			SMA(std::string currencyPair, int history, int seconds);
			virtual ~SMA();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const SMA* other = dynamic_cast<const SMA*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getSMA() const { return sma; }
			// slightly faster falloff
			double getSMA2() const { return sma2; }
			// MA of absolute value, faster falloff
			double getSMA2Abs() const { return sma2abs; }
		private:
			std::string currencyPair;
			int history;
			int seconds;
			double sma;
			double sma2;
			double sma2abs;
		};

	};
};
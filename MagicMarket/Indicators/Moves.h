#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		// https://en.wikipedia.org/wiki/Average_directional_movement_index
		class Moves : public Base
		{
		public:
			virtual void reset() override;
			Moves(std::string currencyPair, int history, int seconds);
			virtual ~Moves();

			virtual void declareExports() const override;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const Moves* other = dynamic_cast<const Moves*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			// for ADX
			double getPlusDMMA()  const { return plusDMMA; }
			double getMinusDMMA() const { return minusDMMA; }
			// for RSI
			double getUpMA()   const { return upMA; }
			double getDownMA() const { return downMA; }
			// for TSI
			double getMomentumMA() const { return momentumMA; }
			double getAbsoluteMomentumMA() const { return momentumAbsMA; }
		private:
			std::string currencyPair;
			int seconds;
			int history;

			double plusDMMA;
			double minusDMMA;
			double plusDMMA_pushed;
			double minusDMMA_pushed;

			double upMA;
			double downMA;
			double upMA_pushed;
			double downMA_pushed;

			double momentumMA;
			double momentumAbsMA;
			double momentumMA_pushed;
			double momentumAbsMA_pushed;

			// Refresh the moving averages only when the time for one period is passed.
			std::time_t lastMAPush;
		};

	};
};
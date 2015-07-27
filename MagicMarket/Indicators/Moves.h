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
			Moves(std::string currencyPair, int history, int seconds);
			virtual ~Moves();

			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const Moves* other = dynamic_cast<const Moves*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getPlusDMMA()  { return plusDMMA; }
			double getMinusDMMA() { return minusDMMA; }

		private:
			std::string currencyPair;
			int seconds;
			int history;

			double plusDMMA;
			double minusDMMA;
		};

	};
};
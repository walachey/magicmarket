#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class Moves;
		class ATR;

		// https://en.wikipedia.org/wiki/Average_directional_movement_index
		class ADX : public Base
		{
		public:
			ADX(std::string currencyPair, int history, int seconds);
			virtual ~ADX();

			virtual void declareExports() const override;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const ADX* other = dynamic_cast<const ADX*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getpDIMA() const { return pDIMA; }
			double getmDIMA() const { return mDIMA; }
			double getADX() const { return adx; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double pDIMA;
			double mDIMA;
			double adx;

			Moves *moves;
			ATR *atr;
		};

	};
};
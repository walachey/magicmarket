#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		// http://www.investopedia.com/terms/s/stochasticoscillator.asp
		class StochasticOscillator : public Base
		{
		public:
			virtual void reset() override;
			StochasticOscillator(std::string currencyPair, int history, int seconds);
			virtual ~StochasticOscillator();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const StochasticOscillator* other = dynamic_cast<const StochasticOscillator*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getPercentK() const { return percentK; }
			double getPercentD() const { return percentD; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double percentK;
			double percentD;
		};

	};
};
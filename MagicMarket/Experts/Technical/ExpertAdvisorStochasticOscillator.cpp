#include "ExpertAdvisorStochasticOscillator.h"

#include <algorithm>

#include "Indicators/StochasticOscillator.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorStochasticOscillator::ExpertAdvisorStochasticOscillator()
	{
		oscillator = Indicators::get<Indicators::StochasticOscillator>("EURUSD", 14, ONEMINUTE);
		lastPercentD = lastPercentK = std::numeric_limits<double>::quiet_NaN();
	}


	ExpertAdvisorStochasticOscillator::~ExpertAdvisorStochasticOscillator()
	{
	}

	void ExpertAdvisorStochasticOscillator::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double &percentK = oscillator->getPercentK();
		const double &percentD = oscillator->getPercentD();

		if (std::isnan(percentK) || std::isnan(percentD)) return;

		const QuantLib::Decimal margin = 25.0;
		const QuantLib::Decimal range = 50.0 - margin;

		const double totalAmplitude = std::max(0.0, (std::abs(percentD - 50.0) - margin) / range);
		const int crossover = Math::checkCrossover(lastPercentD, percentD, lastPercentK, percentK);

		if (crossover == +1 && percentD < 50)
		{
			setMood(+1.0, totalAmplitude);
		}
		else if (crossover == -1 && percentD > 50)
		{
			setMood(-1.0, totalAmplitude);
		}
		else
		{
			setMood(0.0, 1.0);
		}
		
		lastPercentD = percentD;
		lastPercentK = percentK;
	}

};
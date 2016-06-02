#include "ExpertAdvisorMultiCurrency.h"

#include <iomanip>

#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"
#include "Statistics.h"


namespace MM
{
	ExpertAdvisorMultiCurrency::ExpertAdvisorMultiCurrency()
	{
	}


	ExpertAdvisorMultiCurrency::~ExpertAdvisorMultiCurrency()
	{
	}

	void ExpertAdvisorMultiCurrency::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		double mood(0.0), certainty(0.0);

		int signSum = 0;
		double actualAverage = 0.0;

		for (const Variable & var : variables)
		{
			const double val = var.get();
			actualAverage += val;

			const int sign = Math::signum(val);
			signSum += sign;
		}

		actualAverage /= static_cast<double>(variables.size());
		const int requiredSigns = variables.size();
		if (signSum >= requiredSigns)
		{
			mood = +1.0;
		}
		else if (signSum <= -requiredSigns)
		{
			mood = -1.0;
		}

		if (mood != 0.0) certainty = 1.0;

		setMood(mood, certainty);
	}

	void ExpertAdvisorMultiCurrency::afterExportsDeclared()
	{
		for (const Variable & var : statistics.getVariables())
		{
			if (var.name.find("MC_tf") != 0) continue;
			variables.push_back(var);
		}
	}
};
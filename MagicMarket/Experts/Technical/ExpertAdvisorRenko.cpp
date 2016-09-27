#include "ExpertAdvisorRenko.h"

#include <iomanip>

#include "Indicators/Renko.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorRenko::ExpertAdvisorRenko(double sensitivity)
	{
		renko = Indicators::get<Indicators::Renko>("EURUSD", 20, sensitivity * ONEPIP);
	}


	ExpertAdvisorRenko::~ExpertAdvisorRenko()
	{
	}

	void ExpertAdvisorRenko::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const size_t N = 10;
		std::vector<double> bars = renko->getBars(N);
		if (bars.empty()) return;

		// No action when no trend reversal has happened.
		if (bars[N - 1] == bars[N - 2])
			return setMood(0.0, 0.0);

		// When a trend reversal happened, weight it according to how strong the previous trend was.
		const double mean = (Math::sum(bars) - bars[N - 1]) / static_cast<double>(N - 1);
		double mood = +1;
		if (bars[N - 1] < 0.0) mood = -1;

		const double certainty = std::abs(mean - bars[N - 1]) / 2.0;

		setMood(mood, certainty);
	}

};
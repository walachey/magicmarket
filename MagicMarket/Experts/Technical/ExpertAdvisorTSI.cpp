#include "ExpertAdvisorTSI.h"

#include <iomanip>

#include "Indicators/TSI.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorTSI::ExpertAdvisorTSI()
	{
		tsi = Indicators::get<Indicators::TSI>("EURUSD", 14, 2 * ONEMINUTE);
	}


	ExpertAdvisorTSI::~ExpertAdvisorTSI()
	{
	}

	void ExpertAdvisorTSI::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double &tsiValue = tsi->getTSI();

		if (std::isnan(tsiValue)) return;

		const QuantLib::Decimal margin = 50.0;
		const QuantLib::Decimal range = 100.0 - margin;

		const QuantLib::Decimal certainty = (std::abs(tsiValue) - margin) / range;
		if (tsiValue > margin)
			return setMood(-1.0, 0.5 + 0.5 * certainty);
		if (tsiValue < -margin)
			return setMood(+1.0, 0.5 + 0.5 * certainty);
		setMood(0.0, 0.25);
	}

};
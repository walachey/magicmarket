#include "ExpertAdvisorCCI.h"

#include <iomanip>

#include "Indicators/CCI.h"
#include "Indicators/SMA.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorCCI::ExpertAdvisorCCI()
	{
		cciShort = Indicators::get<Indicators::CCI>("EURUSD", 28, ONEMINUTE);
		cciLong  = Indicators::get<Indicators::CCI>("EURUSD", 7, 5 * ONEMINUTE);

		cciShortMA = Indicators::get<Indicators::SMA>("", 10, 0, std::bind(&Indicators::CCI::getCCI, cciShort));
		cciLongMA  = Indicators::get<Indicators::SMA>("", 10, 0, std::bind(&Indicators::CCI::getCCI, cciLong));

		cciShortMA->setCustomDescription("CCI(28, 60) ");
		cciLongMA-> setCustomDescription("CCI(7, 5 * 60) ");
	}


	ExpertAdvisorCCI::~ExpertAdvisorCCI()
	{
	}

	void ExpertAdvisorCCI::reset()
	{
	}

	void ExpertAdvisorCCI::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double cci1 = cciShort->getCCI();
		const double cci2 = cciLong->getCCI();

		if (std::isnan(cci1) || std::isnan(cci2)) return;

		const QuantLib::Decimal margin = 100.0;

		double action = 0.0;
		double confidence = 0.25;
		double marginDistance = 0.0;

		if (cci1 < -margin && cci2 < -margin)
		{
			action = +1.0;
			marginDistance = std::abs(cci2 + margin);
		}
		else if (cci1 > margin && cci2 > margin)
		{
			action = -1.0;
			marginDistance = std::abs(cci2 - margin);
		}

		if (action != 0.0)
		{
			assert(marginDistance >= 0.0);
			confidence = marginDistance / (0.25 * margin);
			confidence = Math::clamp(confidence, 0.25, 1.0);
		}

		setMood(action, confidence);
	}

};
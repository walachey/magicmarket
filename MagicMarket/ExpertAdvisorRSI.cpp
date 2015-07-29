#include "ExpertAdvisorRSI.h"

#include <iomanip>

#include "Indicators/RSI.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorRSI::ExpertAdvisorRSI()
	{
		rsiShort = static_cast<Indicators::RSI*>(Indicators::RSI("EURUSD", 28, ONEMINUTE).init());
		rsiLong  = static_cast<Indicators::RSI*>(Indicators::RSI("EURUSD", 14, 5 * ONEMINUTE).init());
	}


	ExpertAdvisorRSI::~ExpertAdvisorRSI()
	{
	}

	void ExpertAdvisorRSI::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double &rsi1 = rsiShort->getRSI();
		const double &rsi2 = rsiLong->getRSI();

		if (std::isnan(rsi1) || std::isnan(rsi2)) return;

		const QuantLib::Decimal avgRSI = (rsi1 + rsi2) / 2.0;

		const QuantLib::Decimal margin = 30.0;

		market.updateParameter("RSI", avgRSI);

		if (avgRSI > (100.0 - margin))
			return setMood(-1.0, 0.5 + 0.5 * (avgRSI - (100.0 - margin)) / margin);
		if (avgRSI < (margin))
			return setMood(+1.0, 0.5 + 0.5 * (1.0 - (avgRSI / margin)));
	}

};
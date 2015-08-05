#include "ExpertAdvisorRSI.h"

#include <iomanip>

#include "Indicators/RSI.h"
#include "Indicators/SMA.h"
#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorRSI::ExpertAdvisorRSI()
	{
		rsiShort = Indicators::get<Indicators::RSI>("EURUSD", 28, ONEMINUTE);
		rsiLong  = Indicators::get<Indicators::RSI>("EURUSD", 15, 5 * ONEMINUTE);

		rsiShortMA = Indicators::get<Indicators::SMA>("", 10, 0, std::bind(&Indicators::RSI::getRSI, rsiShort));
		rsiLongMA  = Indicators::get<Indicators::SMA>("", 10, 0, std::bind(&Indicators::RSI::getRSI, rsiLong));
	}


	ExpertAdvisorRSI::~ExpertAdvisorRSI()
	{
	}

	void ExpertAdvisorRSI::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double rsi1 = rsiShort->getRSI();
		const double rsi2 = rsiLong->getRSI();

		if (std::isnan(rsi1) || std::isnan(rsi2)) return;

		const double rsi1MA = rsiShortMA->getSMA();
		const double rsi2MA = rsiLongMA->getSMA();

		if (std::isnan(rsi1MA) || std::isnan(rsi2MA)) return;

		QuantLib::Decimal avgRSI   = (rsi1   + rsi2)   / 2.0;
		QuantLib::Decimal avgRSIMA = (rsi1MA + rsi2MA) / 2.0;

		const QuantLib::Decimal margin = 30.0;

		market.updateParameter("RSI", avgRSI);

		double action = 0.0;
		double confidence = 0.25;

		if (avgRSI > margin && avgRSIMA < margin)
		{
			action = +1.0;
		}
		else if (avgRSI < margin && avgRSIMA > margin)
		{
			action = -1.0;
			// mirror the curve
			avgRSI = 100.0 - avgRSI;
			avgRSIMA = 100.0 - avgRSIMA;
		}

		if (action != 0.0)
		{
			assert(avgRSI >= avgRSIMA);
			confidence = 4.0 * (avgRSI - avgRSIMA) / margin;
			confidence = Math::clamp(confidence, 0.25, 1.0);
		}

		setMood(action, confidence);
	}

};
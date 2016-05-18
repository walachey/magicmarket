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

		rsiShortMA->setCustomDescription("RSI(28, 60) ");
		rsiLongMA-> setCustomDescription("RSI(15, 5 * 60) ");
	}


	ExpertAdvisorRSI::~ExpertAdvisorRSI()
	{
	}

	void ExpertAdvisorRSI::reset()
	{
	}

	void ExpertAdvisorRSI::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		const double rsi1 = rsiShort->getRSI();
		const double rsi2 = rsiLong->getRSI();

		if (std::isnan(rsi1) || std::isnan(rsi2)) return;

		const QuantLib::Decimal margin = 20.0;

		double action = 0.0;
		double confidence = 0.25;
		double marginDistance = 0.0;

		if (rsi1 < margin && rsi2 < margin)
		{
			action = +1.0;
			marginDistance = rsi2;
		}
		else if (rsi1 > 100.0 - margin && rsi2 >  100.0 - margin)
		{
			action = -1.0;
			marginDistance = 100.0 - rsi2;
		}

		if (action != 0.0)
		{
			assert(marginDistance >= 0.0);
			assert(marginDistance <= margin);
			confidence = (margin - marginDistance) / margin;
			confidence = Math::clamp(confidence, 0.25, 1.0);
		}

		setMood(action, confidence);
	}

};
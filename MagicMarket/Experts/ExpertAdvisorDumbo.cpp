#include "ExpertAdvisorDumbo.h"
#include "Market.h"
#include "Trade.h"
#include "Stock.h"

#include <algorithm>

const int START_HOUR = 10;
const int END_HOUR = 20;


namespace MM
{
	ExpertAdvisorDumbo::ExpertAdvisorDumbo()
	{
	}


	ExpertAdvisorDumbo::~ExpertAdvisorDumbo()
	{
	}



	void ExpertAdvisorDumbo::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		//30 min frequency
		//if (time % (60 * 30) != 0)
		//	return;

		std::string currencyPair = "EURUSD";

		std::tm *tm = std::gmtime(&time);

		//Only trade between 10am und 8pm
		if (tm->tm_hour < START_HOUR || tm->tm_hour >= END_HOUR)
		{
			say("outta time - fork u");
			return;
		}

		Stock *stock = market.getStock(currencyPair);
		TimePeriod pips = stock->getTimePeriod(time - 30 * ONEMINUTE, time);
		PossibleDecimal close(pips.getClose()), open(pips.getOpen());

		if (!close || !open) return;

		QuantLib::Decimal iOpenCloseDif = *close - *open;
		market.updateParameter("m30D", iOpenCloseDif / ONEPIP);

		QuantLib::Decimal magicNumber = 12.0 * ONEPIP;
		QuantLib::Decimal confidenceFactor = iOpenCloseDif / magicNumber;
		confidenceFactor = std::min(1.0, std::max(-1.0, confidenceFactor));

		double mood = 0.0;
		if (confidenceFactor > 0.25)
			mood = -1.0;
		else if (confidenceFactor < -0.25)
			mood = +1.0;
		else
			confidenceFactor = 0.25f; // for mood = 0.0
		setMood(mood, std::abs(confidenceFactor));

		if (confidenceFactor >= 1.0f)
			say("" + currencyPair + " I just sold.");
		else if (confidenceFactor <= -1.0f)
			say("" + currencyPair + " I just bought.");
	}


	void ExpertAdvisorDumbo::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		
	}

}; // namespace MM
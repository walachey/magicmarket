#include "ExpertAdvisorDumbo.h"
#include "Market.h"
#include "Trade.h"
#include "Stock.h"

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



	void ExpertAdvisorDumbo::execute(const std::time_t &secondsSinceStart, const QuantLib::Date &date, const std::time_t &time)
	{
		//30 min frequency
		if (secondsSinceStart % (60 * 30) != 0)
			return;

		std::string currencyPair = "EURUSD";

		std::tm *tm = std::gmtime(&time);

		//Only trade between 10am und 8pm
		if (tm->tm_hour < START_HOUR || tm->tm_hour >= END_HOUR)
		{
			say("outta time - fork u");
			return;
		}

		Stock *stock = market.getStock(currencyPair);
		TimePeriod pips = stock->getTimePeriod(time - 30, time);

		int iOpenCloseDif = pips.getClose - pips.getOpen;

		if (iOpenCloseDif > 120)
		{
			Trade *trade = market.newTrade(Trade::Sell(currencyPair, 0.01));
			say("New trade: I just sold " + currencyPair);
		}

		if (iOpenCloseDif < 120)
		{
			Trade *trade = market.newTrade(Trade::Buy(currencyPair, 0.01));
			say("New trade: I just bought " + currencyPair);
		}


	}


	void ExpertAdvisorDumbo::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		
	}

}; // namespace MM
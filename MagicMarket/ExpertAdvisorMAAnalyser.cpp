#include "ExpertAdvisorMAAnalyser.h"
#include "Market.h"
#include "Trade.h"
#include "Stock.h"

const int PERIOD = 10;
const int COUNT = 10;
std::vector<float> v;

namespace MM
{
	ExpertAdvisorMAAnalyser::ExpertAdvisorMAAnalyser()
	{
	}


	ExpertAdvisorMAAnalyser::~ExpertAdvisorMAAnalyser()
	{
	}



	void ExpertAdvisorMAAnalyser::execute(const std::time_t &secondsSinceStart, const QuantLib::Date &date, const std::time_t &time)
	{
		std::string currencyPair = "EURUSD";
		std::tm *tm = std::gmtime(&time);

		Stock *stock = market.getStock(currencyPair);
		TimePeriod pips = stock->getTimePeriod(time - 30, time);

		//Calc and store moving average
		float mv = 0;
		for (int i = 0; i < COUNT; i++)
		{
			mv += *(stock->getTimePeriod(time - PERIOD * (i + 1), time - PERIOD * i).getClose());
		}

		if (secondsSinceStart % (60 * PERIOD) == 0)
		{
			v.push_back(mv / COUNT);
		}
		else
		{
			v.back() = mv / COUNT;
		}

		if (v.size() < 10)
			return;

		if (v.size() > 10)
			v.erase(v.begin());


		//check for other trades
		int buyCount = 0;
		int sellCount = 0;
		std::vector<Trade*> &currentTrades = market.getOpenTrades();

		for (Trade *& trade : currentTrades)
		{
			if (trade->currencyPair != currencyPair) continue;
			if (trade->type == Trade::T_BUY) buyCount++;
			if (trade->type == Trade::T_SELL) sellCount++;
		}

		float sum = 0;

		for (int i = 9; i > 7; i++)
			sum += v.at(i) - v.at(i - 1);
			
		//this is where the magic happens - dif between last 3 ma's > 50?
		if (sum > 0.0050)
		{
			setMood(1, 95);
			if (buyCount == 0)
			{
				Trade *trade = market.newTrade(Trade::Buy(currencyPair, 0.01));
				say("New trade: I just bought " + currencyPair);
			}
		}
		else if (sum < -0.0050)
		{
			setMood(-1, 95);
			if (sellCount == 0)
			{
				Trade *trade = market.newTrade(Trade::Sell(currencyPair, 0.01));
				say("New trade: I just sold " + currencyPair);
			}

		}
		else
			setMood(0, 0);

	}


	void ExpertAdvisorMAAnalyser::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{

	}

}; // namespace MM
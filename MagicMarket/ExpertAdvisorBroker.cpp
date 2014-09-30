#include "ExpertAdvisorBroker.h"

#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"
#include "Trade.h"

#include <algorithm>

namespace MM
{

	ExpertAdvisorBroker::ExpertAdvisorBroker()
	{
		lastExecutionActionTime = 0;
	}


	ExpertAdvisorBroker::~ExpertAdvisorBroker()
	{
	}

	void ExpertAdvisorBroker::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		if (currencyPair != "EURUSD") return;

		// cooldown!
		if ((lastExecutionActionTime != 0) && (time < lastExecutionActionTime + ONEMINUTE)) return;

		std::vector<ExpertAdvisor*> &experts = market.getExperts();
		float avgMood = 0.0f;
		float certaintySum = 0.0f;
		float avgCertaintyNormalized = 0.0f;
		int expertCount = 0;

		for (ExpertAdvisor *& expert : experts)
		{
			if (expert == this) continue;
			if (expert->getLastCertainty() == 0.0f) continue;

			avgMood += expert->getLastCertainty() * expert->getLastMood();
			avgCertaintyNormalized += expert->getLastCertainty() * expert->getLastMood();
			certaintySum += expert->getLastCertainty();
			expertCount += 1;
		}

		if (certaintySum != 0.0f)
			avgMood /= certaintySum;
		float avgCertainty = 0.0f;
		if (expertCount != 0)
		{
			avgCertainty = std::abs(avgCertaintyNormalized) / (float)expertCount;
		}

		const float confidenceMargin = 0.4f;
		float ownCertainty = std::min(1.0f, avgCertainty / confidenceMargin);
		const float unanimityMargin = 0.25; // if the experts are very confident but all say different things - nope...
		float ownMood = 0.0f;
		if (avgMood <= -unanimityMargin) ownMood = -1.0f;
		else if (avgMood >= +unanimityMargin) ownMood = +1.0f;
		setMood(ownMood, ownCertainty);

		// okay, now execute whatever shit we need to satisfy our raving bunch of experts
		if (ownCertainty < 1.0) return;
		if (ownMood == 0.0f) return;
		
		// let's do this shit
		Trade::Type type = (avgMood > 0.0f) ? Trade::T_BUY : Trade::T_SELL;
		std::string action = (type == Trade::T_BUY) ? "buy" : "sell";
		// but only if we don't already have 2 trades of that type pending
		int existingTradeCount = 0;
		std::vector<Trade*> &currentTrades = market.getOpenTrades();

		for (Trade *& trade : currentTrades)
		{
			if (trade->currencyPair != currencyPair) continue;
			if (trade->type != type) continue;
			++existingTradeCount;
		}

		if (existingTradeCount >= 2)
		{
			
			std::ostringstream os; os << "Would " << action << " but there are already " << existingTradeCount << " trades.";
			say(os.str());
			return;
		}

		// execute this mofo
		lastExecutionActionTime = time;

		QuantLib::Decimal orderPrice = 0.0;
		Stock *stock = market.getStock(currencyPair);
		if (!stock)
		{
			say("No stock data.");
			return;
		}
		TimePeriod period = stock->getTimePeriod(time);
		if (type == Trade::T_BUY)
			period.setValueFunction(&Tick::getAsk);
		else period.setValueFunction(&Tick::getBid);
		PossibleDecimal close = period.getClose();
		if (!close)
		{
			say("Missing close data.");
			return;
		}

		Trade trade;
		trade.currencyPair = currencyPair;
		trade.lotSize = 0.014;
		trade.type = type;
		trade.orderPrice = *close;
		market.newTrade(trade);

		std::ostringstream os; os << "@" << currencyPair << " executed " << action << " !!";
		say(os.str());
	}
};
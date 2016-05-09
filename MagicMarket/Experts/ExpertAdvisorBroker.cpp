#include "ExpertAdvisorBroker.h"

#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"
#include "Trade.h"

#include <algorithm>

namespace MM
{
	void ExpertAdvisorBroker::reset()
	{
		lastExecutionActionTime = 0;
	}

	
	ExpertAdvisorBroker::ExpertAdvisorBroker()
	{
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
		double avgMood = 0.0;
		double certaintySum = 0.0;
		double avgCertaintyNormalized = 0.0;
		int expertCount = 0;

		for (ExpertAdvisor *& expert : experts)
		{
			if (expert == this) continue;
			if (expert->getLastCertainty() == 0.0) continue;
			if (!expert->isExecutive()) continue;

			avgMood += expert->getLastCertainty() * expert->getLastMood();
			avgCertaintyNormalized += expert->getLastCertainty() * expert->getLastMood();
			certaintySum += expert->getLastCertainty();
			expertCount += 1;
		}
		assert(expertCount <= 1);

		if (certaintySum != 0.0)
			avgMood /= certaintySum;
		double avgCertainty = 0.0;
		if (expertCount != 0)
		{
			avgCertainty = std::abs(avgCertaintyNormalized) / static_cast<double>(expertCount);
		}

		const double confidenceMargin = 0.4;
		double ownCertainty = std::min(1.0, avgCertainty / confidenceMargin);
		const double unanimityMargin = 0.25; // if the experts are very confident but all say different things - nope...
		double ownMood = 0.0;
		if (avgMood <= -unanimityMargin) ownMood = -1.0;
		else if (avgMood >= +unanimityMargin) ownMood = +1.0;
		setMood(ownMood, ownCertainty);

		// okay, now execute whatever shit we need to satisfy our raving bunch of experts
		if (ownCertainty < 1.0) return;
		if (ownMood == 0.0) return;
		
		// let's do this shit
		Trade::Type type = (avgMood > 0.0f) ? Trade::T_BUY : Trade::T_SELL;
		std::string action = (type == Trade::T_BUY) ? "buy" : "sell";
		// but only if we don't already have 2 trades of that type pending
		int existingTradeCount = 0;
		int otherTypeTradeCount = 0;
		std::vector<Trade*> &currentTrades = market.getOpenTrades();

		for (Trade *& trade : currentTrades)
		{
			if (trade->currencyPair != currencyPair) continue;
			if (trade->type != type)
				++otherTypeTradeCount;
			else ++existingTradeCount;
		}

		if (existingTradeCount >= 2 && otherTypeTradeCount == 0)
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

		QuantLib::Decimal lotSize = 0.05 * Math::clamp(avgCertainty, 0.25, 1.0);

		Trade trade;
		trade.currencyPair = currencyPair;
		trade.lotSize = lotSize;
		trade.type = type;
		trade.orderPrice = *close;
		trade.setStopLossPrice(trade.orderPrice + ONEPIP * 5.0 * ((type == Trade::T_BUY) ? -1.0 : +1.0));
		market.newTrade(trade);

		std::ostringstream os; os << "@" << currencyPair << " executed " << action << " !!";
		say(os.str());
	}
};
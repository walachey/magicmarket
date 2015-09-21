#include "ExpertAdvisorLimitAdjuster.h"
#include "Market.h"
#include "Statistics.h"
#include "Trade.h"
#include "Stock.h"

namespace MM
{

	void ExpertAdvisorLimitAdjuster::reset()
	{
		hourOfDay = -1;
	}

	ExpertAdvisorLimitAdjuster::ExpertAdvisorLimitAdjuster()
	{
	}


	ExpertAdvisorLimitAdjuster::~ExpertAdvisorLimitAdjuster()
	{
	}

	void ExpertAdvisorLimitAdjuster::declareExports() const
	{
		ExpertAdvisor::declareExports();

		statistics.addVariable(Variable("hour_of_day", [&](){ return static_cast<double>(this->hourOfDay); }, "Hour of the trading day (GMT)."));
	}


	void ExpertAdvisorLimitAdjuster::execute(const std::time_t &secondsSinceStart, const std::time_t &timestamp)
	{
		std::tm *time = std::gmtime(&timestamp);
		hourOfDay = static_cast<int>(time->tm_hour);
	}


	void ExpertAdvisorLimitAdjuster::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		std::string message = "";
		int badTradeCount = 0;
		// check all open trades and see if we can adjust the panic limit
		std::vector<Trade*> &trades = market.getOpenTrades();
		//for (Trade *&trade : trades)
		for (std::vector<Trade*>::iterator iter = trades.begin(); iter != trades.end(); ++iter)
		{
			Trade *&trade = *iter;
			Stock * stock = market.getStock(trade->currencyPair);
			if (!stock)
			{
				message = "I can't judge " + trade->currencyPair;
				continue;
			}
			
			TimePeriod pips = stock->getTimePeriod(time);
			if (trade->type == Trade::T_BUY)
				pips.setValueFunction(&Tick::getBid);
			else pips.setValueFunction(&Tick::getAsk);

			PossibleDecimal closePos = pips.getClose();
			if (closePos == nullptr)
			{
				message = "No close data for " + trade->currencyPair;
				continue;
			}

			QuantLib::Decimal profitPips = *closePos - trade->orderPrice;
			if (trade->type == Trade::T_SELL) profitPips = trade->orderPrice - *closePos;
			if (profitPips > 2.0 * ONEPIP)
			{
				assert(profitPips >= 0.0);
				QuantLib::Decimal difference = 0.5 * profitPips;
				QuantLib::Decimal stopLoss = trade->orderPrice + difference;
				if (trade->type == Trade::T_SELL) stopLoss = trade->orderPrice - difference;

				bool improvement = trade->getStopLossPrice() == 0.0 || (
					((trade->type == Trade::T_BUY) && (trade->getStopLossPrice() < stopLoss))
					|| ((trade->type == Trade::T_SELL) && (trade->getStopLossPrice() > stopLoss)));
				
				if (improvement)
				{
					trade->setStopLossPrice(stopLoss);
					market.updateTrade(trade);
					std::ostringstream os; os << "@" << trade->currencyPair << "/" << *closePos << " set SL/" << stopLoss;
					say(os.str());
				}
				else
					++badTradeCount;
			}

			// now check enforcement of current limits
			bool closeTrade = false;
			if (trade->type == Trade::T_SELL && 
				(((trade->getStopLossPrice() != 0.0) && ((*closePos - ONEPIP) > trade->getStopLossPrice()))
				|| ((trade->getTakeProfitPrice() != 0.0) && ((*closePos + ONEPIP) < trade->getTakeProfitPrice()))))
				closeTrade = true;
			if (trade->type == Trade::T_BUY && 
				(((trade->getStopLossPrice() != 0.0) && ((*closePos + ONEPIP) < trade->getStopLossPrice()))
				|| ((trade->getTakeProfitPrice() != 0.0) && ((*closePos - ONEPIP) > trade->getTakeProfitPrice()))))
				closeTrade = true;

			if (closeTrade)
			{
				std::ostringstream os; os << "@" << trade->currencyPair << " Closing Trade! PIPS: " << int(profitPips / ONEPIP);
				say(os.str());
				market.closeTrade(trade);
			}
		}

		if (message.empty() && badTradeCount > 0)
		{
			std::ostringstream os;
			os << "There are " << badTradeCount << " trades with no improvement.";
			message = os.str();
		}

		if (!message.empty())
			say(message);
	}

	bool ExpertAdvisorLimitAdjuster::acceptNewTrade(Trade *newTrade) 
	{
		// close all trades on the same symbol in the opposite direction
		std::vector<Trade*> toClose;
		for (Trade *&trade : market.getOpenTrades())
		{
			if (newTrade->currencyPair != trade->currencyPair) continue;
			if (newTrade->type == trade->type) continue;

			toClose.push_back(trade);
		}

		for (Trade *&trade : toClose)
		{
			market.closeTrade(trade);
		}

		if (!toClose.empty())
		{
			std::ostringstream os; os << "@I closed " << toClose.size() << " fail trade" << ((toClose.size() == 1) ? "" : "s") << "!";
			say(os.str());
		}

		// only accept new trades in good market hours
		if (hourOfDay < 11 || hourOfDay > 15) return false;
		return true;
	}

}; // namespace MM
#include "ExpertAdvisorLimitAdjuster.h"
#include "Market.h"
#include "Trade.h"
#include "Stock.h"

namespace MM
{
	ExpertAdvisorLimitAdjuster::ExpertAdvisorLimitAdjuster()
	{
	}


	ExpertAdvisorLimitAdjuster::~ExpertAdvisorLimitAdjuster()
	{
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

				bool improvement = trade->stopLossPrice == 0.0 || (
					((trade->type == Trade::T_BUY) && (trade->stopLossPrice < stopLoss))
					|| ((trade->type == Trade::T_SELL) && (trade->stopLossPrice > stopLoss)));
				
				if (improvement)
				{
					trade->stopLossPrice = stopLoss;
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
				(((trade->stopLossPrice != 0.0) && ((*closePos - ONEPIP) > trade->stopLossPrice))
				|| ((trade->takeProfitPrice != 0.0) && ((*closePos + ONEPIP) < trade->takeProfitPrice))))
				closeTrade = true;
			if (trade->type == Trade::T_BUY && 
				(((trade->stopLossPrice != 0.0) && ((*closePos + ONEPIP) < trade->stopLossPrice))
				|| ((trade->takeProfitPrice != 0.0) && ((*closePos - ONEPIP) > trade->takeProfitPrice))))
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

}; // namespace MM
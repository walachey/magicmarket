#include "VirtualMarket.h"

#include <WinSock2.h>
#include <SimpleIni.h>

#include "Helpers.h"
#include "Market.h"
#include "Trade.h"
#include "Tick.h"
#include "TradingDay.h"

MM::VirtualMarket *virtualMarket = nullptr;

namespace MM
{
	void VirtualMarket::checkInit()
	{
		{ // scope
			CSimpleIniA ini;
			ini.LoadFile("market.ini");

			if (ini.IsEmpty())
			{
				std::cout << "ERROR: Could not load configuration!" << std::endl;
			}

			std::istringstream is(ini.GetValue("Virtual Market", "Enabled", "0"));
			bool enabled;
			is >> enabled;

			if (!enabled) return;
		}

		virtualMarket = new VirtualMarket();
		virtualMarket->init();
	}


	void VirtualMarket::init()
	{
		CSimpleIniA ini;
		ini.LoadFile("market.ini");

		int day, month, year;
		std::string dayString = ini.GetValue("Virtual Market", "Day", "2014-01-30");
		sscanf_s(dayString.c_str(), "%d-%d-%d", &year, &month, &day);
		date = QuantLib::Date(day, (QuantLib::Month)(month), year);

		// update market to match settings
		market.setVirtual(true);
		market.setSleepDuration(0);

		// load tick data
		tradingDay = new TradingDay(date, market.getStock("EURUSD", true));
		tradingDay->loadFromFile();
		tickIndex = 0;

		std::vector<std::string> secondary = { "USDDKK", "USDCHF", "EURCAD", "EURAUD", "EURJPY", "AUDCHF" };
		for (std::string &pair : secondary)
		{
			secondaryCurrencies.push_back(new TradingDay(date, market.getStock(pair, true)));
			secondaryCurrencies.back()->loadFromFile();
		}
	}

	VirtualMarket::VirtualMarket()
	{
		tradeCounter = 1000;
		lastTick = nullptr;
		totalProfitPips = 0.0;
		wonTrades = lostTrades = 0;
	}


	VirtualMarket::~VirtualMarket()
	{
	}

	void VirtualMarket::evaluate()
	{
		std::cout << "--------------VIRTUAL MARKET DAY--------------" << std::endl;
		std::cout << "EVALUATION -------------------(profit in pips)" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "ONLY CLOSED TRADES----------------------------" << std::endl;
		std::cout << "\tTRADES WON\t" << wonTrades << std::endl;
		std::cout << "\tTRADES LOST\t" << lostTrades << std::endl;
		std::cout << "\tTOTAL PROFIT\t" << totalProfitPips << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << "INCLUDING " << trades.size() << " OPEN TRADES-----------------------" << std::endl;
		std::cout << "----------------------------------------------" << std::endl;

		for (Trade &trade : trades)
			evaluateTrade(trade);

		std::cout << "\tTRADES WON\t" << wonTrades << std::endl;
		std::cout << "\tTRADES LOST\t" << lostTrades << std::endl;
		std::cout << "\tTOTAL PROFIT\t" << totalProfitPips << std::endl;

		getchar();
		exit(1);
	}

	void VirtualMarket::execute()
	{
		// market is stop when no more ticks are available
		if (tickIndex >= tradingDay->ticks.size())
		{
			evaluate();
			return;
		}
		// send the general state of the market BEFORE the ticks
		publishGeneralInfo();

		// send next tick
		std::time_t previousTime = 0;
		if (lastTick) previousTime = lastTick->getTime();

		lastTick = &tradingDay->getTickByIndex(tickIndex++);
		sendTickMsg(lastTick, tradingDay);
		
		// send all ticks of secondary currencies up to the time of the leading currency
		for (TradingDay *&day : secondaryCurrencies)
		{
			for (Tick &tick : day->ticks)
			{
				if (tick.getTime() <= previousTime) continue;
				if (tick.getTime() > lastTick->getTime()) break;
				sendTickMsg(&tick, day);
			}
		}
	}

	void VirtualMarket::publishGeneralInfo()
	{
		// account info message gets abused for the profit stuff
		std::ostringstream accountInfo;
		accountInfo << "account VM " << 0 << " " << totalProfitPips << " " << (double)wonTrades << " " << (double)lostTrades;
		market.send(accountInfo.str());

		// publish all currently open trades in the same json format as the metatrader
		std::ostringstream orderString;
		bool first = true;
		for (Trade &trade : trades)
		{
			if (!first) orderString << ",";
			else first = false;
/*
	this madness is done to enable simple copy & paste from the metatrader code in case anything ever changes
*/

#define OrderSymbol trade.currencyPair
#define OrderType ((trade.type == Trade::T_BUY) ? 0 : 1)
#define OrderTicket trade.ticketID
#define OrderOpenPrice trade.orderPrice
#define OrderTakeProfit trade.takeProfitPrice
#define OrderStopLoss trade.stopLossPrice
#define OrderOpenTime 0
#define OrderExpiration 0
#define OrderLots trade.lotSize
#define OrderProfit (trade.getProfitAtTick(*lastTick) / ONEPIP)
			orderString << "{\"pair\":\"" << OrderSymbol << "\", \"type\":" << OrderType << ", \"ticket_id\":" << OrderTicket << ", \"open_price\":" << OrderOpenPrice << ", \"take_profit\":" << OrderTakeProfit << ", \"stop_loss\":" << OrderStopLoss << ", \"open_time\":" << OrderOpenTime << ", \"expire_time\":" << OrderExpiration << ", \"lots\":" << OrderLots << ", \"profit\":" << OrderProfit << "}";
#undef OrderSymbol
#undef OrderType
#undef OrderTicket
#undef OrderOpenPrice
#undef OrderTakeProfit
#undef OrderStopLoss
#undef OrderOpenTime
#undef OrderExpiration
#undef OrderLots
#undef OrderProfit
		}

		market.send(std::string("orders VM [") + orderString.str() + "]");
	}

	void VirtualMarket::sendTickMsg(Tick *tick, TradingDay *day)
	{
		std::ostringstream msg;
		msg << "tick VM " << day->getCurrencyPair() << " " << tick->getBid() << " " << tick->getAsk() << " " << tick->getTime();
		market.send(msg.str());
	}

	void VirtualMarket::onReceive(const std::string &message)
	{
		assert(message.substr(0, 3) == "cmd");

		std::istringstream is(message);
		std::string accountInfo, command;
		is >> accountInfo >> command;

		if (command == "set")
		{
			int tradeType;
			std::string pair;
			Trade trade;
			is >> tradeType >> trade.currencyPair >> trade.orderPrice >> trade.stopLossPrice >> trade.takeProfitPrice >> trade.lotSize;
			
			trade.type = (tradeType == 0) ? Trade::T_BUY : Trade::T_SELL;
			trade.ticketID = ++tradeCounter;
			trade.removeSaveFile();
			trades.push_back(trade);
		}
		else if (command == "unset")
		{
			int ticketID;
			is >> ticketID;
			
			for (auto iter = trades.begin(); iter != trades.end(); ++iter)
			{
				Trade &trade = *iter;
				if (trade.ticketID != ticketID) continue;
				evaluateTrade(trade);
				trade.removeSaveFile();
				trades.erase(iter);
				break;
			}
		}
	}

	void VirtualMarket::evaluateTrade(const Trade &trade)
	{
		if (trade.currencyPair != tradingDay->getCurrencyPair()) return;

		QuantLib::Decimal profit = trade.getProfitAtTick(*lastTick);
		profit /= ONEPIP;
		
		if (profit > 0.0) wonTrades += 1;
		else if (profit < 0.0) lostTrades += 1;
		totalProfitPips += profit;
	}
};
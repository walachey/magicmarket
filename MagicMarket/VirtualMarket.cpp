#include "VirtualMarket.h"

#include <WinSock2.h>

#include <SimpleIni.h>

#include "Helpers.h"
#include "Market.h"
#include "Trade.h"
#include "Tick.h"
#include "TradingDay.h"
#include "Stock.h"
#include "ExpertAdvisor.h"

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

		std::string hourString = ini.GetValue("Virtual Market", "Hours", "0-0");
		sscanf_s(hourString.c_str(), "%d-%d", &fromHour, &toHour);

		isSilent = ini.GetValue("Virtual Market", "Silent", "0") == std::string("1");

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

		// fast forward to start of market day
		if (fromHour > 0)
		{
			for (; tickIndex < tradingDay->ticks.size(); ++tickIndex)
			{
				Tick &tick = tradingDay->getTickByIndex(tickIndex);
				std::time_t time = tick.getTime();
				std::tm *tm = std::gmtime(&time);
				if (tm->tm_hour < fromHour)
				{
					lastTick = &tick;
					continue;
				}
				break;
			}
		}

		std::cout << "------------VIRTUAL MARKET SETUP--------(" << fromHour << "-" << toHour << ")" << std::endl;
		std::cout << "\tTOTAL TICK COUNT\t" << tradingDay->ticks.size() << std::endl;
		std::cout << "\tSTARTING AT TICK\t" << tickIndex << std::endl;
		std::cout << "\tFIRST TICK AT\t" << timeToString(tradingDay->ticks.front().getTime()) << std::endl;
		std::cout << "\tLAST TICK AT\t" << timeToString(tradingDay->ticks.back().getTime()) << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}

	VirtualMarket::VirtualMarket()
	{
		tradeCounter = 1000;
		lastTick = nullptr;
		totalProfitPips = 0.0;
		wonTrades = lostTrades = 0;
		fromHour = toHour = 0;
	}


	VirtualMarket::~VirtualMarket()
	{
	}

	void VirtualMarket::evaluate()
	{
		std::cout << "\n------------VIRTUAL MARKET RESULTS------------" << std::endl;
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

		evaluateMood();

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

		// check end of market day
		if ((toHour > 0) && (previousTime != 0))
		{
			std::tm *tm = std::gmtime(&previousTime);
			printf("\r\tCURRENT TIME IS %02d:%02d", tm->tm_hour, tm->tm_min);
			if (tm->tm_hour > toHour)
			{
				// fast forward to end
				tickIndex = tradingDay->ticks.size();
			}
		}

		takeMoodSnapshot();
	}

	void VirtualMarket::publishGeneralInfo()
	{
		// account info message gets abused for the profit stuff
		std::ostringstream accountInfo;
		accountInfo << "A VM " << 0 << " " << totalProfitPips << " " << (double)wonTrades << " " << (double)lostTrades;
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

		market.send(std::string("O VM [") + orderString.str() + "]");
	}

	void VirtualMarket::sendTickMsg(Tick *tick, TradingDay *day)
	{
		std::ostringstream msg;
		msg << "T VM " << day->getCurrencyPair() << " " << tick->getBid() << " " << tick->getAsk() << " " << tick->getTime();

		int probabilityToSend = 1;
		if (day != tradingDay) probabilityToSend = 0;
		market.send(msg.str(), probabilityToSend);
	}

	void VirtualMarket::onReceive(const std::string &message)
	{
		assert(message[0] == 'C');

		std::istringstream is(message);
		std::string accountInfo, command;
		is >> accountInfo >> accountInfo >> command;

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

	void VirtualMarket::proxySend(const std::string &message)
	{
		pendingMessages.push(message);
	}

	std::string VirtualMarket::proxyReceive()
	{
		if (pendingMessages.empty()) return "";
		std::string value = pendingMessages.front();
		pendingMessages.pop();
		return value;
	}

	void VirtualMarket::takeMoodSnapshot()
	{
		if (!lastTick) return;

		std::time_t time = lastTick->getTime();
		Stock *stock = tradingDay->stock;

		int lookaheadTime = ONEHOUR;
		std::time_t endTime = time + lookaheadTime;
		if (endTime > tradingDay->ticks.back().getTime()) return;

		TimePeriod period = stock->getTimePeriod(time, endTime);

		PossibleDecimal open, high, low;
		open = period.getOpen();
		high = period.getHigh();
		low = period.getLow();
		if (!open) return;
		assert(open && high && low);

		QuantLib::Decimal highDiff = *high - *open;
		QuantLib::Decimal lowDiff = *open - *low;

		const QuantLib::Decimal optimumValue = 20.0 * ONEPIP;
		moodFunctions["BUY"].push_back(std::min(highDiff / optimumValue, 1.0));
		moodFunctions["SELL"].push_back(-std::min(lowDiff / optimumValue, 1.0));

		// now assess all experts, too
		for (ExpertAdvisor *&expert : market.getExperts())
		{
			moodFunctions[expert->getName()].push_back(expert->getLastCertainty() * expert->getLastMood());
		}
	}

	void VirtualMarket::evaluateMood()
	{
		std::vector<double> &BUY = moodFunctions["BUY"];
		std::vector<double> &SELL = moodFunctions["SELL"];

		const double stddevBuy = Math::stddev(BUY);
		const double stddevSell = Math::stddev(SELL);

		std::cout << "\nMOOD EVALUATION-------------------------------" << std::endl;
		for (auto iter = moodFunctions.begin(); iter != moodFunctions.end(); ++iter)
		{
			const std::string &name = iter->first;
			const std::vector<double> &values = iter->second;
			
			std::vector<double> buySide = Math::covarVec(values, BUY);
			std::vector<double> sellSide = Math::covarVec(values, SELL);
			std::vector<double> total =  Math::max(buySide, sellSide);

			
			double stddev = Math::stddev(values);
			double var = std::pow(Math::stddev(values), 2.0);

			double buySideCovar = Math::sum(buySide) / (double)(buySide.size() - 1);
			double buySideCorr = buySideCovar / (stddev * stddevBuy);
			
			double sellSideCovar = Math::sum(sellSide) / (double)(sellSide.size() - 1);
			double sellSideCorr = sellSideCovar / (stddev * stddevSell);

			if (!isnormal(buySideCovar)) continue;

			double accuracy = Math::accuracy(values, BUY, SELL);

			printf_s("* %-7s\tBUY %+4d%%\tSELL %+4d%%\tACCU %+4d%%\n", name.c_str(), int(100.0 * buySideCorr + 0.5), int(100.0 * sellSideCorr + 0.5), int(100.0 * accuracy + 0.5));
			//std::cout << "\t\tstdbuy " << stddevBuy << "\tstdsell " << stddevSell << "\tstdsignal " << stddev << std::endl;
		}

		// save for python evaluation
		Debug::serialize(moodFunctions, "saves/moodfun.json");
	}
};
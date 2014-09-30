#pragma once

#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>
#include <string>
#include <queue>
#include <map>

#include "Trade.h"

namespace MM
{
	class TradingDay;
	class Tick;

	class VirtualMarket
	{
	public:
		VirtualMarket();
		~VirtualMarket();

		
		static void checkInit();
		void evaluate();

		// called by the market
		void onReceive(const std::string &message);
		void execute();
		// this bypasses the normal central station system
		void proxySend(const std::string &message);
		std::string proxyReceive();

		// config
		bool isSilent;
	private:
		std::queue<std::string> pendingMessages;

		void init();
		void evaluateTrade(const Trade &trade);
		int tradeCounter;
		std::vector<Trade> trades;
		
		void sendTickMsg(Tick *tick, TradingDay *day);
		void publishGeneralInfo();

		// this is always the leading currency
		TradingDay *tradingDay;
		size_t tickIndex;
		Tick *lastTick;
		// the secondary currencies follow the timing of the primary one
		std::vector<TradingDay*> secondaryCurrencies;

		// config
		QuantLib::Date date;
		int fromHour, toHour;
		

		// evaluation and statistics
		QuantLib::Decimal totalProfitPips;
		int wonTrades, lostTrades;
		void takeMoodSnapshot();
		void evaluateMood();
		std::map<std::string, std::vector<double>> moodFunctions;
	};


};

extern MM::VirtualMarket *virtualMarket;
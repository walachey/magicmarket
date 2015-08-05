#pragma once

#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <ctime>
#include "Trade.h"

namespace MM
{
	class TradingDay;
	class Tick;

	struct VirtualTradeMetaInfo
	{
		Trade::Type type;
		std::time_t openingTime;
		int snapshotIndex;
		double profit;

		std::time_t closingTime;
		int closingSnapshotIndex;
		VirtualTradeMetaInfo(Trade::Type type, std::time_t openingTime, int snapshotIndex) :
			type(type),
			openingTime(openingTime),
			snapshotIndex(snapshotIndex)
		{
			profit = 0.0;
			closingTime = 0;
			closingSnapshotIndex = -1;
		}

		void VirtualTradeMetaInfo::setClosed(double profit, std::time_t closingTime, int closingSnapshotIndex)
		{
			this->profit = profit;
			this->closingTime = closingTime;
			this->closingSnapshotIndex = closingSnapshotIndex;
		}
	};

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

		// for easier merging of data sets
		int getLastSnapshotIndex() { return snapshotIndex; }
	private:
		std::queue<std::string> pendingMessages;

		void init();
		void evaluateTrade(const Trade &trade);
		int tradeCounter;
		std::vector<Trade> trades;
		std::map<int32_t, VirtualTradeMetaInfo> tradesMetaInfo;
		
		void sendTickMsg(const Tick *tick, TradingDay *day);
		void publishGeneralInfo();

		// this is always the leading currency
		TradingDay *tradingDay;
		size_t tickIndex;
		Tick *lastTick;
		// the secondary currencies follow the timing of the primary one
		std::vector<TradingDay*> secondaryCurrencies;
		std::vector<std::vector<Tick>::iterator> secondaryCurrenciesIterators;
		// config
		QuantLib::Date date;
		int fromHour, toHour;
		

		// evaluation and statistics
		QuantLib::Decimal totalProfitPips;
		int wonTrades, lostTrades;
		void takeMoodSnapshot();
		int snapshotIndex; // for easier evaluation of other data
		void evaluateMood();
		void evaluateTrades();
		std::map<std::string, std::vector<double>> moodFunctions;
	};


};

extern MM::VirtualMarket *virtualMarket;
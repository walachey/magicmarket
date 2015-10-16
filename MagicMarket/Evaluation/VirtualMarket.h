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

	namespace vm
	{
		class Profiler;
	};

	struct VirtualTradeMetaInfo
	{
		Trade::Type type;
		std::time_t openingTime;
		double profit;

		std::time_t closingTime;
		// whether the trade was automatically closed at the end of a day
		int forcefulClose;
		VirtualTradeMetaInfo(Trade::Type type, std::time_t openingTime) :
			type(type),
			openingTime(openingTime),
			forcefulClose(0)
		{
			profit = 0.0;
			closingTime = 0;
		}

		void VirtualTradeMetaInfo::setClosed(double profit, std::time_t closingTime, int closingSnapshotIndex, bool forceful = false)
		{
			this->profit = profit;
			this->closingTime = closingTime;
			this->forcefulClose = forceful ? 1 : 0;
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

	private:
		std::queue<std::string> pendingMessages;

		void init();
		void evaluateTrade(const Trade &trade, bool forceful = false);
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
		struct _config
		{
			QuantLib::Date datePeriodBegin, datePeriodEnd;
			QuantLib::Date date;
			int fromHour, toHour;
		} config;
		

		void evaluateDay();
		void advanceDay(bool noevaluation = false);
		void cleanUpDayData();
		void prepareDayData();
		bool isOutOfPeriod();

		// evaluation and statistics
		struct _estimation
		{
			double buyCertainty;
			double sellCertainty;
			double currentLeadingPrice;

			_estimation() : buyCertainty(0.0), sellCertainty(0.0), currentLeadingPrice(0.0) {}
		} currentEstimation;

		struct _results
		{
			QuantLib::Decimal totalProfitPips;
			double wonTrades, lostTrades;

			// IO stuff
			bool tradesHeaderPrinted;
			_results() : totalProfitPips(0.0), wonTrades(0.0), lostTrades(0.0), tradesHeaderPrinted(false){}
		} results;

		vm::Profiler *profiler;
		void predictTradeEfficiency();
		void saveTrades();
	};


};

extern MM::VirtualMarket *virtualMarket;
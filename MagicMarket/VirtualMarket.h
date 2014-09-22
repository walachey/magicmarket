#pragma once

#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>
#include <string>

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

	private:
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


		QuantLib::Decimal totalProfitPips;
		int wonTrades, lostTrades;
		// config
		QuantLib::Date date;
	};


};

extern MM::VirtualMarket *virtualMarket;
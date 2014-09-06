#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <map>

#include <ql/time/date.hpp>

#include "Tick.h"
#include "TimePeriod.h"

namespace MM
{     
	class Trade;
	class TradingDay;

	class Stock
	{
	public:
		Stock(std::string pair);
		~Stock();

		Trade *newTrade(Trade trade);

		std::string getCurrencyPair() { return currencyPair; }

		void receiveFreshTick(Tick tick);

		TradingDay* getTradingDay(QuantLib::Date date, bool allowCreation=false);

		// time period handling
		static_assert(sizeof(std::time_t) == 8,"fuck");
		TimePeriod getTimePeriod(const std::time_t &start, const std::time_t &end);
		TimePeriod getTimePeriod(const std::time_t &time);

		// saving and loading
		std::string getDirectoryName();
		static std::string getDirectoryName(std::string currencyPair);

	private:
		std::map<QuantLib::Date, TradingDay*> tradingDays;
		std::string currencyPair;
	};


};
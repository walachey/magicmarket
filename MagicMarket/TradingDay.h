#pragma once

#include <map>
#include <string>
#include <vector>

#include <ql/time/date.hpp>

#include "Tick.h"

namespace MM
{
	class Stock;


	class TradingDay
	{
	public:
		TradingDay(QuantLib::Date date, Stock *stock);
		~TradingDay();

		TradingDay *getPreviousDay();
		TradingDay *getNextDay();

		QuantLib::Date getDate() { return date; }
		
		void receiveFreshTick(Tick tick);

		// saving & loading
		std::ostream& getSaveFile();
		bool loadFromFile();
		std::string getSavePath();
		static std::string getSavePath(Stock* stock, QuantLib::Date forDate);
		std::string getSaveFileName();
		static std::string getSaveFileName(QuantLib::Date forDate);
	private:
		QuantLib::Date date;
		std::vector<Tick> ticks;
		std::fstream *saveFile;
		Stock *stock;

		std::vector<Tick> & getTicks() { return ticks; }

		friend class TimePeriod;
	};


};
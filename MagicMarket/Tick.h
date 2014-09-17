#pragma once

#include <ctime>
#include <ostream>
#include <iostream>


#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include "Helpers.h"

namespace MM
{

	class Tick
	{
	public:
		Tick();
		~Tick();

		QuantLib::Decimal getBid() { return bid; }
		QuantLib::Decimal getAsk() { return ask; }
		QuantLib::Decimal getMid() { return (bid + ask) / QuantLib::Decimal(2); }
		QuantLib::Decimal getSpread() { return (ask - bid); }

		QuantLib::Date getDate() { return dateFromTime(time); };
		std::time_t getTime() { return time; }
		std::time_t *getTimeP() { return &time; }

		std::string toString();

	private:
		std::time_t time;
		QuantLib::Decimal bid, ask;


	public:
		friend class Market;
		friend class Stock;
		friend class TradingDay;

		friend std::ostream& operator<< (std::ostream &out, MM::Tick &tick);
		friend std::istream& operator>> (std::istream &in, MM::Tick &tick);
		int getOutputBitSize();
	};

	std::ostream& operator<< (std::ostream &out, MM::Tick &tick);
	std::istream& operator>> (std::istream &in, MM::Tick &tick);
};



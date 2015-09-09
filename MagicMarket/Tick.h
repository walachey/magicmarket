#pragma once

#include <ctime>
#include <ostream>
#include <iostream>


#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include "Helpers.h"

namespace MM
{

	namespace io
	{
		class DataReader;
	};

	class Tick
	{
	public:
		Tick();
		~Tick();

		QuantLib::Decimal getBid() const { return bid; }
		QuantLib::Decimal getAsk() const { return ask; }
		QuantLib::Decimal getMid() const { return (bid + ask) / QuantLib::Decimal(2); }
		QuantLib::Decimal getSpread() const { return (ask - bid); }

		QuantLib::Date getDate() const { return dateFromTime(time); };
		std::time_t getTime() const { return time; }
		std::time_t *getTimeP() { return &time; }

		std::string toString();

	private:
		std::time_t time;
		QuantLib::Decimal bid, ask;


	public:
		friend class Market;
		friend class Stock;
		friend class TradingDay;
		friend class io::DataReader;

		friend std::ostream& operator<< (std::ostream &out, const MM::Tick &tick);
		friend std::istream& operator>> (std::istream &in, const MM::Tick &tick);
		int getOutputBitSize() const;
	};

	std::ostream& operator<< (std::ostream &out, const MM::Tick &tick);
	std::istream& operator>> (std::istream &in, const MM::Tick &tick);
};



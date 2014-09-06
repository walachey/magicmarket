#pragma once

#include <ctime>
#include <ostream>
#include <iostream>
#include <memory>

#include <ql/time/date.hpp>
#include <ql/types.hpp>

#include "Tick.h"

namespace MM
{
	typedef std::shared_ptr<QuantLib::Decimal> PossibleDecimal;

	class Stock;

	class TimePeriod
	{
	public:
		TimePeriod(Stock *stock_, const std::time_t &startTime_, const std::time_t &endTime_, QuantLib::Decimal(Tick::*valueFunction_)());
		TimePeriod(const TimePeriod &other);
		~TimePeriod();

		void setValueFunction(QuantLib::Decimal(Tick::*fun)());

		// accessors
		
		PossibleDecimal TimePeriod::getClose();

	private:
		QuantLib::Decimal (Tick::*valueFunction)();
		Stock *stock;
		std::time_t startTime;
		std::time_t endTime;
	};
};

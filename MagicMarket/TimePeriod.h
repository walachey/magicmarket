#pragma once

#include <ctime>
#include <ostream>
#include <iostream>
#include <memory>
#include <vector>

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

		// manipulation of the time period
		// seconds can be negative or positive
		bool expandStartTime(int seconds);
		bool expandEndTime(int seconds);

		// accessors
		PossibleDecimal getClose();
		PossibleDecimal getOpen();
		PossibleDecimal getHigh();
		PossibleDecimal getLow();
		PossibleDecimal getAverage();
		int getMaximumSecondsBetweenTicks();

		std::vector<double> toVector(int secondsInterval);

	private:
		QuantLib::Decimal (Tick::*valueFunction)();
		Stock *stock;
		std::time_t startTime;
		std::time_t endTime;
	};
};

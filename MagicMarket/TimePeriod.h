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
		TimePeriod(Stock *stock_, const std::time_t &startTime_, const std::time_t &endTime_, QuantLib::Decimal(Tick::*valueFunction_)() = nullptr);
		TimePeriod(const TimePeriod &other);
		~TimePeriod();

		void setValueFunction(QuantLib::Decimal(Tick::*fun)());

		// manipulation of the time period
		// seconds can be negative or positive
		bool expandStartTime(int seconds);
		bool expandEndTime(int seconds);
		// shifts the whole time period to the right (positive) or left (negative)
		bool shift(int seconds);
		// sets the time period to point to another stock
		bool setStock(std::string currencyPair);
		bool setStock(Stock *stock);

		// accessors
		PossibleDecimal getClose();
		PossibleDecimal getOpen();
		PossibleDecimal getHigh();
		PossibleDecimal getLow();
		PossibleDecimal getAverage();
		int getMaximumSecondsBetweenTicks();

		std::vector<double> toVector(int secondsInterval);

		std::time_t getStartTime() const { return startTime; }
		std::time_t getEndTime() const { return endTime; }
		std::time_t getDuration() const { return endTime - startTime; }
	private:
		QuantLib::Decimal (Tick::*valueFunction)();
		Stock *stock;
		std::time_t startTime;
		std::time_t endTime;
	};
};

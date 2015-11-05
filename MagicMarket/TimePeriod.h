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
	class TradingDay;

	class TimePeriod
	{
	public:
		TimePeriod(Stock *stock_, const std::time_t &startTime_, const std::time_t &endTime_, QuantLib::Decimal(Tick::*valueFunction_)() const);
		TimePeriod(const TimePeriod &other);
		~TimePeriod();

		void setValueFunction(QuantLib::Decimal(Tick::*fun)()const);

		// manipulation of the time period
		// seconds can be negative or positive
		bool expandStartTime(int seconds);
		bool expandEndTime(int seconds);
		// shifts the whole time period to the right (positive) or left (negative)
		bool shift(int seconds);
		// sets the time period to point to another stock
		bool setStock(std::string currencyPair);
		bool setStock(Stock *stock);
		// virtual market speciality: set a certain trading day (which might already have all the future information).
		// That way, you'll have to jump through hoops to incorporate future data points.
		void setTradingDay(TradingDay *day);

		// accessors
		PossibleDecimal getClose();
		PossibleDecimal getOpen();
		PossibleDecimal getHigh();
		PossibleDecimal getLow();
		PossibleDecimal getAverage();
		int getMaximumSecondsBetweenTicks(int *totalTicks = nullptr, int *totalChanges = nullptr);

		const Tick *getLastTick();

		std::vector<double> toVector(int secondsInterval);

		std::time_t getStartTime() const { return startTime; }
		std::time_t getEndTime() const { return endTime; }
		std::time_t getDuration() const { return endTime - startTime; }
	private:
		QuantLib::Decimal(Tick::*valueFunction)() const;
		// Normally, the time period is created from a stock.
		Stock *stock;
		// In certain circumstances (virtual market) you can also force a specific trading day instance.
		TradingDay *tradingDay;
		std::time_t startTime;
		std::time_t endTime;

		// cache functionality - for faster access
		std::vector<Tick>::iterator ticksBegin, ticksEnd, ticksTotalBegin, ticksTotalEnd;
		bool cacheDirty;
		bool checkInitCache();
		bool isCacheGood() { return !cacheDirty; }

		std::vector<Tick>::iterator begin() { return ticksBegin; }
		std::vector<Tick>::iterator end() { return ticksEnd; }
	};
};

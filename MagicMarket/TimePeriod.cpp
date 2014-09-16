#include "TimePeriod.h"
#include "Tick.h"
#include "Stock.h"
#include "TradingDay.h"
#include "Market.h"

#include <assert.h>
#include <algorithm>

namespace MM
{
	TimePeriod::TimePeriod(Stock *stock_, const std::time_t &startTime_, const std::time_t &endTime_, QuantLib::Decimal(Tick::*valueFunction_)()) : stock(nullptr), startTime(0), endTime(0), valueFunction(nullptr)
	{
		stock = stock_;
		startTime = startTime_;
		endTime = endTime_;

		if (valueFunction_ != nullptr)
			valueFunction = valueFunction_;
		else valueFunction = &Tick::getMid;
	}

	TimePeriod::TimePeriod(const TimePeriod &other)
	{
		stock = other.stock;
		startTime = other.startTime;
		endTime = other.endTime;
		valueFunction = other.valueFunction;
	}


	TimePeriod::~TimePeriod()
	{
	}

	void TimePeriod::setValueFunction(QuantLib::Decimal(Tick::*fun)())
	{
		valueFunction = fun;
	}
	
	PossibleDecimal TimePeriod::getHigh()
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return nullptr;

		QuantLib::Decimal max = 0.0;
		int count = 0;

		std::vector<Tick> &ticks = day->getTicks();
		for (Tick &tick : ticks)
		{
			if (tick.getTime() < startTime) continue;
			if (tick.getTime() > endTime) break;

			QuantLib::Decimal value = (tick.*valueFunction)();
			if ((count == 0) || (value > max))
				max = value;
			count += 1;
		}

		if (count == 0) return nullptr;
		return PossibleDecimal(new QuantLib::Decimal(max));
	}

	PossibleDecimal TimePeriod::getLow()
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return nullptr;

		QuantLib::Decimal min = 0.0;
		int count = 0;

		std::vector<Tick> &ticks = day->getTicks();
		for (Tick &tick : ticks)
		{
			if (tick.getTime() < startTime) continue;
			if (tick.getTime() > endTime) break;

			QuantLib::Decimal value = (tick.*valueFunction)();
			if ((count == 0) || (value < min))
				min = value;
			count += 1;
		}

		if (count == 0) return nullptr;
		return PossibleDecimal(new QuantLib::Decimal(min));
	}

	PossibleDecimal TimePeriod::getOpen()
	{
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return nullptr;

		// get first tick before period begin
		std::vector<Tick> &ticks = day->getTicks();
		for (size_t i = 0, ii = ticks.size(); i < ii; ++i)
		{
			if (ticks[i].getTime() < startTime) continue;
			if (i == 0) return nullptr;

			return PossibleDecimal(new QuantLib::Decimal((ticks[i-1].*valueFunction)()));
		}

		return nullptr;
	}

	PossibleDecimal TimePeriod::getClose()
	{
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return nullptr;

		// get last tick before period end - might also be before start
		std::vector<Tick> &ticks = day->getTicks();
		for (std::vector<Tick>::reverse_iterator iter = ticks.rbegin(); iter != ticks.rend(); ++iter)
		{
			Tick &tick = *iter;
			if (tick.getTime() > endTime) continue;
			return PossibleDecimal(new QuantLib::Decimal((tick.*valueFunction)()));
		}

		return nullptr;
	}

	PossibleDecimal TimePeriod::getAverage()
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return nullptr;

		QuantLib::Decimal sum = 0.0;
		int count = 0;

		std::vector<Tick> &ticks = day->getTicks();
		for (Tick &tick : ticks)
		{
			if (tick.getTime() < startTime) continue;
			if (tick.getTime() > endTime) break;
			sum += (tick.*valueFunction)();
			count += 1;
		}

		if (count == 0) return nullptr;
		return PossibleDecimal(new QuantLib::Decimal(sum / QuantLib::Decimal(count)));
	}

	int TimePeriod::getMaximumSecondsBetweenTicks()
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return 0;

		int max = 0;

		std::vector<Tick> &ticks = day->getTicks();
		for (size_t i = 1, len = ticks.size(); i < len; ++i)		
		{
			Tick &current = ticks[i];
			Tick &last = ticks[i - 1];
			int timespan = current.getTime() - last.getTime();

			if (timespan > max) max = timespan;
		}
		return max;
	}

	std::vector<double> TimePeriod::toVector(int secondsInterval)
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return std::vector<double>();

		int totalEntries = (endTime - startTime) / secondsInterval + 1;
		std::vector<double> values;
		values.reserve(totalEntries);

		std::vector<Tick> &ticks = day->getTicks();
		std::time_t currentTime = startTime;
		size_t currentIndex = 0;
		while (currentTime < (endTime + secondsInterval))
		{
			// search right tick for time
			while ((currentIndex < ticks.size()) && (ticks[currentIndex].getTime() < currentTime))
				++currentIndex;
			size_t index = (size_t)std::max((int)currentIndex - 1, 0);
			assert((int)index >= 0 && index < ticks.size());
			Tick &tick = ticks[index];
			values.push_back((tick.*valueFunction)());
			currentTime += secondsInterval;
		}

		return values;
	}

	bool TimePeriod::expandStartTime(int seconds)
	{
		std::time_t newTime = startTime - seconds;
		if (newTime > endTime) return false;
		startTime = newTime;
		return true;
	}

	bool TimePeriod::expandEndTime(int seconds)
	{
		std::time_t newTime = endTime + seconds;
		if (newTime < startTime) return false;
		endTime = newTime;
		return true;
	}

	bool TimePeriod::shift(int seconds)
	{
		startTime += seconds;
		endTime += seconds;
		return true;
	}

	bool TimePeriod::setStock(std::string currencyPair)
	{
		Stock *s = market.getStock(currencyPair);
		if (s == nullptr) return false;
		return setStock(s);
	}

	bool TimePeriod::setStock(Stock *stock)
	{
		this->stock = stock;
		return true;
	}

}; // namespace MM
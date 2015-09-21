#include "TimePeriod.h"
#include "Tick.h"
#include "Stock.h"
#include "TradingDay.h"
#include "Market.h"

#include <assert.h>
#include <algorithm>

namespace MM
{
	TimePeriod::TimePeriod(Stock *stock_, const std::time_t &startTime_, const std::time_t &endTime_, QuantLib::Decimal((Tick::*valueFunction_)()const)) : stock(nullptr), startTime(0), endTime(0), valueFunction(nullptr)
	{
		stock = stock_;
		startTime = startTime_;
		endTime = endTime_;

		if (valueFunction_ != nullptr)
			valueFunction = valueFunction_;
		else valueFunction = &Tick::getMid;

		cacheDirty = true;
	}

	TimePeriod::TimePeriod(const TimePeriod &other)
	{
		stock = other.stock;
		startTime = other.startTime;
		endTime = other.endTime;
		valueFunction = other.valueFunction;

		cacheDirty = true;
	}


	TimePeriod::~TimePeriod()
	{
	}

	bool TimePeriod::checkInitCache()
	{
		if (!cacheDirty) return true;
		// for now, assume we are always evaluating one single day
		assert(dateFromTime(startTime) == dateFromTime(endTime));

		ticksEnd = ticksBegin = std::vector<Tick>::iterator();

		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return false;

		ticksTotalBegin = day->ticks.begin();
		ticksTotalEnd = day->ticks.end();
		ticksBegin = day->ticks.end();
		ticksEnd   = day->ticks.end();

		bool endSet(false);
		std::vector<Tick> &ticks = day->getTicks();
		for (size_t i = ticks.size() - 1; i >= 0; --i)
		{
			const Tick &tick = ticks[i];
			if (!endSet && (tick.getTime() < endTime))
			{
				ticksEnd = std::next(ticks.begin(), i + 1);
				endSet = true;
			}

			if (tick.getTime() < startTime)
			{
				ticksBegin = std::next(ticks.begin(), i + 1);
				break;
			}
		}

		cacheDirty = false;
		return true;
	}


	void TimePeriod::setValueFunction(QuantLib::Decimal(Tick::*fun)()const)
	{
		valueFunction = fun;
	}
	
	PossibleDecimal TimePeriod::getHigh()
	{
		if (!checkInitCache()) return nullptr;

		QuantLib::Decimal max = 0.0;
		int count = 0;

		for (const Tick &tick : *this)
		{
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
		if (!checkInitCache()) return nullptr;

		QuantLib::Decimal min = 0.0;
		int count = 0;

		for (const Tick &tick : *this)
		{
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
		if (!checkInitCache()) return nullptr;
		if (ticksBegin == ticksTotalBegin) return nullptr;
		if (ticksBegin == ticksTotalEnd) return nullptr;

		const Tick & tick = *(ticksBegin - 1);
		return PossibleDecimal(new QuantLib::Decimal((tick.*valueFunction)()));
	}

	PossibleDecimal TimePeriod::getClose()
	{
		if (!checkInitCache()) return nullptr;

		Tick *tick = nullptr;
		if (ticksEnd != ticksTotalEnd)
		{
			tick = &*(ticksEnd - 1);
		}
		else
		{
			TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
			assert(day != nullptr);
			tick = &day->ticks.back();
		}
		
		if (tick == nullptr) return nullptr;
		return PossibleDecimal(new QuantLib::Decimal((tick->*valueFunction)()));
	}

	PossibleDecimal TimePeriod::getAverage()
	{
		if (!checkInitCache()) return nullptr;
		
		QuantLib::Decimal sum = 0.0;
		int count = 0;

		for (Tick &tick : *this)
		{
			assert (tick.getTime() >= startTime);
			assert (tick.getTime() < endTime);
			sum += (tick.*valueFunction)();
			count += 1;
		}

		if (count == 0) return nullptr;
		return PossibleDecimal(new QuantLib::Decimal(sum / QuantLib::Decimal(count)));
	}

	int TimePeriod::getMaximumSecondsBetweenTicks(int *totalTicks, int *totalChanges)
	{
		assert(dateFromTime(startTime) == dateFromTime(endTime)); // for now, assume we are always evaluating one single day
		TradingDay *day = stock->getTradingDay(dateFromTime(endTime));
		if (day == nullptr) return -1;

		std::vector<Tick> &ticks = day->getTicks();

		// initialize max time by last/first tick margin to start/end time
		const int startingGap = static_cast<int>(std::max(0ll, ticks[0].getTime() - startTime));
		const int endingGap   = static_cast<int>(std::max(0ll, endTime - ticks.back().getTime()));
		int max = std::max(startingGap, endingGap);

		if (totalTicks != nullptr) *totalTicks = 0;
		if (totalChanges != nullptr) *totalChanges = 0;

		for (size_t i = 1, len = ticks.size(); i < len; ++i)		
		{
			Tick &current = ticks[i];
			Tick &last = ticks[i - 1];

			if (current.getTime() < startTime || current.getTime() > endTime) continue;
			if (last.getTime() < startTime || last.getTime() > endTime) continue;
			
			int timespan = static_cast<int>(current.getTime() - last.getTime());

			if (timespan > max) max = timespan;
			if (totalTicks != nullptr) ++(*totalTicks);
			if (totalChanges != nullptr && (current.getAsk() != last.getAsk())) ++(*totalChanges);
		}
		return max;
	}

	std::vector<double> TimePeriod::toVector(int secondsInterval)
	{
		if (!checkInitCache()) return{};

		const int totalEntries = static_cast<int>(endTime - startTime) / secondsInterval + 1;
		std::vector<double> values;
		values.reserve(totalEntries);

		std::time_t currentTime = startTime;
		auto currentTick = ticksBegin;
		auto lastTick    = currentTick;
		while (currentTime < (endTime + secondsInterval))
		{
			// search right tick for time
			while ((currentTick != ticksEnd) && (currentTick->getTime() < currentTime))
			{
				lastTick = currentTick;
				++currentTick;
			}
			
			Tick &tick = *lastTick;
			assert(tick.getTime() < currentTime || market.isVirtual());
			values.push_back((tick.*valueFunction)());
			currentTime += secondsInterval;
		}

		return values;
	}

	bool TimePeriod::expandStartTime(int seconds)
	{
		cacheDirty = true;

		std::time_t newTime = startTime - seconds;
		if (newTime > endTime) return false;
		startTime = newTime;
		return true;
	}

	bool TimePeriod::expandEndTime(int seconds)
	{
		cacheDirty = true;

		std::time_t newTime = endTime + seconds;
		if (newTime < startTime) return false;
		endTime = newTime;
		return true;
	}

	bool TimePeriod::shift(int seconds)
	{
		cacheDirty = true;

		startTime += seconds;
		endTime += seconds;
		return true;
	}

	bool TimePeriod::setStock(std::string currencyPair)
	{
		cacheDirty = true;

		Stock *s = market.getStock(currencyPair);
		if (s == nullptr) return false;
		return setStock(s);
	}

	bool TimePeriod::setStock(Stock *stock)
	{
		cacheDirty = true;

		this->stock = stock;
		return true;
	}

}; // namespace MM
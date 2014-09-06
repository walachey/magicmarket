#include "TimePeriod.h"
#include "Tick.h"
#include "Stock.h"
#include "TradingDay.h"

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

}; // namespace MM
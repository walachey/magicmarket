#include "Event.h"

#include <assert.h>

namespace MM
{
	Event::Event(Event::Type type) : type(type)
	{

	}

	Event::Event(Event::Type type, std::string currencyPair, QuantLib::Date date, std::time_t time) :
		type(type), currencyPair(currencyPair), date(date), time(time)
	{
		assert(type == Event::Type::NEW_TICK);
	}

	bool Event::operator==(const Event &other) const
	{
		return (type == other.type)
			&& (currencyPair == other.currencyPair)
			&& (date == other.date)
			&& (time == other.time);
	}

	bool Event::youngerThan(const Event &other) const
	{
		return (type == other.type)
			&& (currencyPair == other.currencyPair)
			&& ((date < other.date) || (time < other.time));
	}

}; // namespace MM
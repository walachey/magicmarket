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


}; // namespace MM
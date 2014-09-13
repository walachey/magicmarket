#pragma once

#include <string>
#include <ctime>

#include <ql/types.hpp>
#include <ql/time/date.hpp>

namespace MM
{

	class Event
	{
	public:
		enum Type
		{
			NEW_TICK,
			TIMER
		};

		Type type;

		Event(Event::Type type);
		Event(Event::Type type, std::string currencyPair, QuantLib::Date date, std::time_t time);
		~Event() {};

		std::string currencyPair;
		QuantLib::Date date;
		std::time_t time;

		bool operator==(const Event &other) const;
		bool youngerThan(const Event &other) const;
	};

};
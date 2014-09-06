#include "Helpers.h"
#include <assert.h>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time)
	{
		std::tm *tm = std::gmtime(&time);
		assert(tm);
		return QuantLib::Date(tm->tm_mday, (QuantLib::Month)(tm->tm_mon + 1), 1900 + tm->tm_year);
	}
};
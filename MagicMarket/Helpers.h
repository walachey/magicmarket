#pragma once

#include <ctime>
#include <stdint.h>
#include <ql/time/date.hpp>
#include <ql/types.hpp>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time);

	// constants
	const QuantLib::Decimal ONEPIP = 0.0001;
	const int ONESECOND = 1;
	const int ONEMINUTE = ONESECOND * 60;
	const int ONEHOUR = ONEMINUTE * 60;
	const int ONEDAY = ONEHOUR * 24;
};
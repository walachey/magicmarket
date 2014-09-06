#pragma once

#include <ctime>
#include <stdint.h>
#include <ql/time/date.hpp>
#include <ql/types.hpp>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time);
	const QuantLib::Decimal ONEPIP = 0.0001;
};
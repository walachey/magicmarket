#pragma once

#include <ctime>
#include <stdint.h>
#include <ql/time/date.hpp>
#include <ql/types.hpp>

#include <vector>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time);
	std::time_t mktime(const QuantLib::Date &date, int hour, int minute, int second);

	// constants
	const QuantLib::Decimal ONEPIP = 0.0001;
	const int ONESECOND = 1;
	const int ONEMINUTE = ONESECOND * 60;
	const int ONEHOUR = ONEMINUTE * 60;
	const int ONEDAY = ONEHOUR * 24;

	namespace Math
	{
		template<typename T> std::vector<T> derive(const std::vector<T> &values);
		template<typename T> T sum(const std::vector<T> &values);
	};
};
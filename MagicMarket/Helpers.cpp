#include "Helpers.h"
#include <assert.h>

#include <functional>
#include <numeric>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time)
	{
		std::tm *tm = std::gmtime(&time);
		assert(tm);
		return QuantLib::Date(tm->tm_mday, (QuantLib::Month)(tm->tm_mon + 1), 1900 + tm->tm_year);
	}

	std::time_t mktime(const QuantLib::Date &date, int hour, int minute, int second)
	{
		std::tm timeinfo;
		timeinfo.tm_isdst = -1; // no info available

		timeinfo.tm_hour = hour;
		timeinfo.tm_min = minute;
		timeinfo.tm_sec = second;
		
		timeinfo.tm_mon = ((int)date.month()) - 1;
		timeinfo.tm_year = date.year() - 1900;
		timeinfo.tm_mday = date.dayOfMonth();

		return std::mktime(&timeinfo);
	}
	
	std::string timeToString(std::time_t time)
	{
		char BUF[64];
		size_t val = std::strftime(BUF, 64, "%H:%I:%M", std::gmtime(&time));
		assert(val != 0);
		return std::string(BUF);
	}

	namespace Math
	{
		template<typename T> std::vector<T> derive(const std::vector<T> &values)
		{
			std::vector<T> returnValues;
			returnValues.reserve(values.size() - 1);

			for (size_t i = 1; i < values.size(); ++i)
			{
				T diff = values[i] - values[i - 1];
				returnValues.push_back(diff);
			}
			return returnValues;
		}

		template<typename T> T sum(const std::vector<T> &values)
		{
			return std::accumulate(values.begin(), values.end(), (T)0.0);
		}

		template std::vector<QuantLib::Decimal> derive<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);

		template QuantLib::Decimal sum<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);
	};
};
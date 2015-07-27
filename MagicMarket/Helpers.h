#pragma once

#include <ctime>
#include <stdint.h>
#include <ql/time/date.hpp>
#include <ql/types.hpp>

#include <vector>
#include <string>
#include <map>

namespace MM
{
	QuantLib::Date dateFromTime(std::time_t time);
	std::time_t mktime(const QuantLib::Date &date, int hour, int minute, int second);
	std::string timeToString(std::time_t time);

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
		template<typename T> T avg(const std::vector<T> &values);
		template<typename T> T stddev(const std::vector<T> &values);
		template<typename T> void normalize(std::vector<T> &values);
		template<typename T> std::vector<T> max(const std::vector<T> &values1, const std::vector<T> &values2);
		template<typename T> std::vector<T> mult(const std::vector<T> &values1, const std::vector<T> &values2);
		template<typename T> std::vector<T> covarVec(const std::vector<T> &values1, const std::vector<T> &values2);
		template<typename T> T covarFac(const std::vector<T> &values1, const std::vector<T> &values2);
		template<typename T> T accuracy(const std::vector<T> &values, const std::vector<T> &upper, const std::vector<T> &lower);
		// moving average
		template<typename T> T MA(const T &oldValue, const T &newValue, const int &history);
	};

	template<typename T> std::vector<float> toFloatVector(const std::vector<T> &values);

	namespace Debug
	{
		void serialize(const std::map<std::string, std::vector<double>> &series, std::string filename);
	};
};
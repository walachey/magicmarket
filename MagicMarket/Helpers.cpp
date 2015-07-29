#include "Helpers.h"
#include <assert.h>

#include <functional>
#include <numeric>
#include <algorithm>

#include <fstream>

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

		template<typename T> T avg(const std::vector<T> &values)
		{
			return sum(values) / (T)values.size();
		}

		template<typename T> std::vector<T> max(const std::vector<T> &values1, const std::vector<T> &values2)
		{
			assert(values1.size() == values2.size());
			std::vector<T> out;
			out.resize(values1.size());
			
			for (size_t i = 0; i < values1.size(); ++i)
			{
				out[i] = std::max(values1[i], values2[i]);
			}
			return out;
		}

		template<typename T> T stddev(const std::vector<T> &values)
		{
			T sum = 0.0;
			T avg = Math::avg(values);
			for (size_t i = 0; i < values.size(); ++i)
				sum += std::pow(values[i] - avg, (T)2.0);
			return std::sqrt(sum / (T)(values.size() - 1));
		}

		template<typename T> void normalize(std::vector<T> &values)
		{
			T max = 0.0;
			for (auto &val : values)
			{
				const T absValue = std::abs(val);
				if (absValue > max) max = absValue;
			}
			if (max == 0.0) return; // nothing to do

			for (auto &val : values)
			{
				val /= max;
				assert(val >= (T)-1.0 && val <= (T)1.0);
			}
		}


		template<typename T> std::vector<T> mult(const std::vector<T> &values1, const std::vector<T> &values2)
		{
			assert(values1.size() == values2.size());
			std::vector<T> out;
			out.resize(values1.size());

			for (size_t i = 0; i < values1.size(); ++i)
			{
				out[i] = values1[i] * values2[i];
			}
			return out;
		}

		template<typename T> std::vector<T> covarVec(const std::vector<T> &values1, const std::vector<T> &values2)
		{
			assert(values1.size() == values2.size());
			std::vector<T> out;
			out.resize(values1.size());

			T avg1 = avg(values1);
			T avg2 = avg(values2);

			for (size_t i = 0; i < values1.size(); ++i)
			{
				out[i] = (values1[i] - avg1) * (values2[i] - avg2);
			}
			return out;
		}

		template<typename T> T covarFac(const std::vector<T> &values1, const std::vector<T> &values2)
		{
			assert(values1.size() == values2.size());
			std::vector<T> vec = covarVec(values1, values2);
			
			return sum(vec) / (T)(vec.size() - 1);
		}

		template<typename T> T accuracy(const std::vector<T> &values, const std::vector<T> &upper, const std::vector<T> &lower)
		{
			assert(values.size() == upper.size() && values.size() == lower.size());
			T possibleSum = 0.0;
			T sum = 0.0;

			for (size_t i = 0; i < values.size(); ++i)
			{
				T possible = std::pow(values[i], (T)2.0);
				T actualUpper = values[i] * std::min(values[i], upper[i]);
				T actualLower = values[i] * std::max(values[i], lower[i]);

				if (values[i] > 0.0)
					sum += actualUpper;
				else if(values[i] < 0.0)
					sum += actualLower;
				else continue;
				possibleSum += possible;
			}

			if (possibleSum == 0.0) return 0.0;
			return sum / possibleSum;
		}

		template<typename T> T MA(const T &oldValue, const T &newValue, const int &history)
		{
			if (std::isnan(oldValue)) return newValue;
			const double historyDouble = static_cast<double>(history);
			return ((historyDouble - 1.0) * oldValue + newValue) / historyDouble;
		}

		template std::vector<QuantLib::Decimal> derive<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);
		template QuantLib::Decimal sum<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);
		template QuantLib::Decimal avg<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);
		template QuantLib::Decimal stddev<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values);
		template float stddev<float>(const std::vector<float> &values);
		template void normalize<double>(std::vector<double> &values);
		template std::vector<QuantLib::Decimal> max<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values1, const std::vector<QuantLib::Decimal> &values2);
		template std::vector<QuantLib::Decimal> mult<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values1, const std::vector<QuantLib::Decimal> &values2);
		template std::vector<QuantLib::Decimal> covarVec<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values1, const std::vector<QuantLib::Decimal> &values2);
		template QuantLib::Decimal covarFac<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values1, const std::vector<QuantLib::Decimal> &values2);
		template QuantLib::Decimal accuracy<QuantLib::Decimal>(const std::vector<QuantLib::Decimal> &values, const std::vector<QuantLib::Decimal> &upper, const std::vector<QuantLib::Decimal> &lower);

		template<double> double MA(const double &oldValue, const double &newValue, const int &history);
	};

	template<typename T> std::vector<float> toFloatVector(const std::vector<T> &values)
	{
		std::vector<float> results;
		results.reserve(values.size());
		for (auto & const value : values)
			results.push_back(value);
		return results;
	}
	template std::vector<float> toFloatVector<double>(const std::vector<double> &values);

	namespace Debug
	{
		void serialize(const std::map<std::string, std::vector<double>> &series, std::string filename)
		{
			std::fstream out(filename.c_str(), std::ios_base::out);
			out << "{\n";
			bool firstPair = true;
			for (auto &pair : series)
			{
				if (firstPair) firstPair = false;
				else out << ",\n";

				out << "\t\"" << pair.first << "\" : [\n\t\t";
				bool first = true;
				for (auto &val : pair.second)
				{
					if (first) first = false;
					else out << ", ";
					out << val;
				}
				out << "\n\t\t]";
			}
			out << "\n}" << std::endl;
			out.close();
		}
	};
};
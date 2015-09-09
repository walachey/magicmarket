#pragma once

#include <string>
#include <vector>
#include <memory>

namespace QuantLib
{
	class Date;
};

namespace MM
{
	class Stock;
	class TradingDay;
	class Tick;

	namespace io
	{
		enum class RawDataType
		{
			unknown = 0,
			bitbucket_original,
			gaincapital
		};

		RawDataType dataTypeFromString(std::string s);

		class DataReader
		{
		public:
			DataReader() {}
			virtual ~DataReader() {}
			virtual std::vector<std::shared_ptr<Stock>> readStocks(std::string filename) = 0;

		protected:
			// Returns (and possibly creates) a new trading day without going through stock's accessors.
			TradingDay * getTradingDay(Stock *stock, const QuantLib::Date &date);
			// Adds a tick to a stock without going through the accessors (and thus without f.e. saving it).
			void addTickToStock(Stock *stock, time_t timestamp, double bid, double ask);
		};

		class DataConverter
		{
		public:
			DataConverter(std::string filename, RawDataType type) : filename(filename), type(type)
			{

			}

			DataConverter(std::string filename, std::string dataType) : DataConverter(filename, dataTypeFromString(dataType)) {}

			bool convert();

		private:
			std::string filename;
			RawDataType type;
		};
	};
};
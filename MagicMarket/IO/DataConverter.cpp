#include "DataConverter.h"
#include "Stock.h"
#include "TradingDay.h"
#include "Helpers.h"

#include <assert.h>
#include <ql/utilities/dataparsers.hpp>

#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
namespace filesystem = std::tr2::sys;

#include <boost/date_time/local_time/local_time.hpp>

namespace MM
{
	namespace io
	{
		TradingDay * DataReader::getTradingDay(Stock *stock, const QuantLib::Date &date)
		{
			if (stock->tradingDays.count(date)) return stock->tradingDays[date];
			stock->tradingDays[date] = new TradingDay(date, stock);
			return stock->tradingDays[date];
		}
		
		void DataReader::addTickToStock(Stock *stock, time_t timestamp, double bid, double ask)
		{
			Tick tick;
			tick.time = static_cast<time_t>(timestamp);
			tick.bid = static_cast<QuantLib::Decimal> (bid);
			tick.ask = static_cast<QuantLib::Decimal> (ask);

			TradingDay *day = getTradingDay(stock, tick.getDate());
			day->ticks.push_back(tick);
		}


		RawDataType dataTypeFromString(std::string s)
		{
			if ("BITBUCKET_ORIGINAL" == s)
				return RawDataType::bitbucket_original;
			if ("GAINCAPITAL" == s)
				return RawDataType::gaincapital;
			return RawDataType::unknown;
		}

		bool DataConverter::convert()
		{
			// splits a string by a delimiter and returns a vector of words
			static auto splitString = [](const std::string &s, char delim) {
				std::stringstream ss(s);
				std::string item;
				std::vector<std::string> elems;
				while (std::getline(ss, item, delim)) {
					elems.push_back(std::move(item));
				}
				return elems;
			};

			static auto splitStringToInts = [](const std::string &s, char delim) {
				std::stringstream ss(s);
				std::string item;
				std::vector<int> elems;
				while (std::getline(ss, item, delim)) {
					std::istringstream is(item);
					int next;
					is >> next;
					elems.push_back(next);
				}
				return elems;
			};

			class BitbucketReader : public DataReader
			{
				std::vector<std::shared_ptr<Stock>> readStocks(std::string filename) override
				{
					std::fstream input(filename, std::ios_base::in);

					std::string line;
					while (std::getline(input, line))
					{
						if (line.empty()) continue;
						std::vector<std::string> columns = splitString(line, ',');
						if (columns.size() < 7) continue;
						QuantLib::Date date = QuantLib::DateParser::parseFormatted(columns[0], "%Y.%m.%d");
						std::vector<int> hourMinute = splitStringToInts(columns[1], ':');
						if (hourMinute.size() < 2) continue;
						int seconds = 0;
						if (hourMinute.size() >= 3) seconds = hourMinute[2];
						std::time_t time = mktime(date, hourMinute[0], hourMinute[1], seconds);


					}

					// todo - this has been deprecated during conception.
					assert(false);
					return {};
				}
			};

			class GaincapitalReader : public DataReader
			{
				std::vector<std::shared_ptr<Stock>> readStocks(std::string filename) override
				{
					unsigned int ticksCounted = 0;
					// Extract the stock symbol from the filename. The names should be well-formatted.
					const std::vector<std::string> filenameParts = splitString(filename, '_');
					std::string currencyPair;
					// Find first two parts that are each 3 characters long.
					for (size_t i = 0; i < filenameParts.size() - 1; ++i)
					{
						const std::string &current = filenameParts[i];
						const std::string &next    = filenameParts[i + 1];
						
						if (current.size() != 3 || next.size() != 3) continue;
						currencyPair = current + next;
						break;
					}
					assert(!currencyPair.empty()); // todo, properly handle error.

					std::shared_ptr<Stock> stock = std::make_shared<Stock>(currencyPair);
					std::fstream input(filename, std::ios_base::in);
					std::string line;
					
					while (std::getline(input, line))
					{
						if (line.empty()) continue;
						std::vector<std::string> columns = splitString(line, ',');
						if (columns.size() < 3) continue;
						// Split timestamp to seconds / fractionals so that we do not need to parse a double.
						const std::string &timestampString = columns[0];
						std::istringstream is(std::string(timestampString.begin(), std::find(timestampString.begin(), timestampString.end(), '.')));
						static_assert(sizeof(time_t) >= sizeof(uint64_t), "time_t does not seem to be 64bit");
						uint64_t timestamp(-1);
						is >> timestamp;

						// Remember this tick!
						addTickToStock(stock.get(), static_cast<time_t>(timestamp), std::stod(columns[1]), std::stod(columns[2]));
						++ticksCounted;
					}

					std::cout << "\tTicks read: " << ticksCounted << std::endl;
					return {stock};
				}
			};

			std::unique_ptr<DataReader> reader = nullptr;

			switch (type)
			{
			case RawDataType::bitbucket_original:
				reader = std::unique_ptr<DataReader>(static_cast<DataReader*>(new BitbucketReader()));
				std::cout << "Using bitbucket reader to read " << filename << std::endl;
				break;
			case RawDataType::gaincapital:
				reader = std::unique_ptr<DataReader>(static_cast<DataReader*>(new GaincapitalReader()));
				std::cout << "Using gaincapital reader to read " << filename << std::endl;
				break;
			default:
				std::cout << "No reader for this file available." << std::endl;
				return false;
			};
			assert(!!reader);
			auto stocks = reader->readStocks(filename);

			for (auto &stock : stocks)
			{
				auto days = stock->getAllTradingDays();

				for (auto &dayData : days)
				{
					TradingDay &day = *dayData.second;

					// possibly clean up old file
					std::string filename = day.getSavePath();
					filesystem::remove_filename(filesystem::path(filename.c_str()));

					for (const Tick& tick : day.getTicks())
					{
						day.serializeTick(tick);
					}
				}
			}

			return true;
		}
	};
};
#include "Stock.h"
#include "Market.h"

#include "Trade.h"
#include "TradingDay.h"

#include <filesystem>
namespace filesystem = std::tr2::sys;

namespace MM
{

	Stock::Stock(std::string pair) : currencyPair(pair)
	{
		
	}


	Stock::~Stock()
	{

	}

	TimePeriod Stock::getTimePeriod(const std::time_t &start, const std::time_t &end)
	{
		return TimePeriod(this, start, end, &Tick::getMid);
	}
#include <cstdlib>

	TimePeriod Stock::getTimePeriod(const std::time_t &time)
	{
		assert(sizeof(time) == 8);
		//TimePeriod period(this, time, time, &Tick::getMid);
		TimePeriod *tp = (TimePeriod*)std::calloc(1, sizeof(TimePeriod));
		return *tp;
		//return TimePeriod(nullptr, 0, 0, nullptr);
	}

	void Stock::receiveFreshTick(Tick tick)
	{
		QuantLib::Date date = tick.getDate();
		// add new day for the tick?
		if (!tradingDays.count(date))
		{
			tradingDays[date] = new TradingDay(date, this);
		}
		tradingDays[date]->receiveFreshTick(tick);
	}


	std::string Stock::getDirectoryName()
	{
		return Stock::getDirectoryName(currencyPair);
	}

	std::string Stock::getDirectoryName(std::string currencyPair)
	{
		filesystem::path path(market.getSaveFolderName());
		if (!filesystem::exists(path))
			filesystem::create_directory(path);
		path /= currencyPair;
		if (!filesystem::exists(path))
			filesystem::create_directory(path);

		return path.string();
	}


	TradingDay * Stock::getTradingDay(QuantLib::Date date, bool allowCreation)
	{
		// if the trading day is already loaded, just return it
		if (tradingDays.count(date)) return tradingDays[date];

		// otherwise, try to load the day from file
		std::string pathString = TradingDay::getSavePath(this, date);
		filesystem::path path(pathString);
		if (!filesystem::exists(path) && !allowCreation) return nullptr;

		// exists, so it can possibly contain stock data
		tradingDays[date] = new TradingDay(date, this);
		tradingDays[date]->loadFromFile();

		return tradingDays[date];
	}
};
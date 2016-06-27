#include "VirtualMarket.h"

#include <WinSock2.h>

#include <regex>
#include <filesystem>
namespace filesystem = std::tr2::sys;

#include <SimpleIni.h>

#include "EnvironmentVariables.h"
#include "Helpers.h"
#include "Market.h"
#include "Statistics.h"
#include "Trade.h"
#include "Tick.h"
#include "TradingDay.h"
#include "Stock.h"
#include "ExpertAdvisor.h"
#include "IO/DataConverter.h"
#include "IO/KeyValueDB.h"
#include "VM/Profiler.h"


MM::VirtualMarket *virtualMarket = nullptr;

namespace MM
{
	void VirtualMarket::checkInit()
	{
		{ // scope
			CSimpleIniA ini;
			ini.LoadFile("market.ini");

			if (ini.IsEmpty())
			{
				std::cout << "ERROR: Could not load configuration!" << std::endl;
			}

			std::istringstream is(ini.GetValue("Virtual Market", "Enabled", "0"));
			bool enabled;
			is >> enabled;

			if (!enabled) return;
		}

		virtualMarket = new VirtualMarket();
		virtualMarket->init();
	}


	void VirtualMarket::init()
	{
		CSimpleIniA ini;
		ini.LoadFile("market.ini");

		// Init profiler.
		profiler = new vm::Profiler();
		profiler->init(static_cast<void*>(&ini));

		// Possibly convert some data files first.
		io::KeyValueDB db("saves/virtual_market.datafiles");
		size_t skipped = 0;

		for (size_t i = 0; i < 10; ++i)
		{
			const std::string configName = std::string("Virtual Market Data ") + std::to_string(i + 1);
			const bool isRegexp = ini.GetLongValue(configName.c_str(), "Regexp", 0) == 1;
			std::string baseFilename = ini.GetValue(configName.c_str(), "Filename", "");
			if (baseFilename.empty()) continue;

			std::vector<std::string> filenames;

			if (!isRegexp) filenames = { baseFilename };
			else
			{
				// Get all files in the directory.
				const std::string directory = ini.GetValue(configName.c_str(), "Directory", ".");
				// That matches the given filename-regexp.
				std::regex filePattern(baseFilename);

				filesystem::directory_iterator iter(directory), end;
				for (; iter != end; ++iter)
				{
					const filesystem::path current = *iter;
					const std::string currentName = current.filename().string();
					if (!std::regex_match(currentName, filePattern)) continue;
					
					filenames.push_back(directory + "/" + currentName);
				}
			}

			for (const std::string &filename : filenames)
			{
				// Check if already converted once.
				if (db.get(filename) == "1")
				{
					skipped += 1;
					continue;
				}
				std::cout << "Data file '" << filename << "': \t\tconverting file.." << std::endl;
				std::string filetype = ini.GetValue(configName.c_str(), "Filetype", "");
				// Try to read the file
				io::DataConverter converter(filename, filetype);
				if (converter.convert() == true)
				{
					db.put(filename, "1");
				}
				else
				{
					std::cout << "\t! Reading Data failed!" << std::endl;
				}
			}
		}

		if (skipped > 0)
			std::cout << "Skipped " << skipped << " data input files." << std::endl;

		auto readDate = [&](std::string key)
		{
			int day, month, year;
			std::string dayString = ini.GetValue("Virtual Market", key.c_str(), "2014-01-30");
			sscanf_s(dayString.c_str(), "%d-%d-%d", &year, &month, &day);
			return QuantLib::Date(day, (QuantLib::Month)(month), year);
		};
		config.datePeriodBegin = readDate("Begin");
		config.datePeriodEnd   = readDate("End");
		config.date            = config.datePeriodBegin;

		// And set date as environment variable.
		environmentVariables.set("YEAR", environmentVariables.get("YEAR") + std::to_string(config.datePeriodBegin.year()));

		std::string hourString = ini.GetValue("Virtual Market", "Hours", "0-0");
		sscanf_s(hourString.c_str(), "%d-%d", &config.fromHour, &config.toHour);
		
		isSilent = ini.GetValue("Virtual Market", "Silent", "0") == std::string("1");
		config.waitOnFinished = ini.GetValue("Virtual Market", "WaitOnFinished", "0") == std::string("1");

		// update market to match settings
		market.setVirtual(true);
		market.setSleepDuration(0);

		// feed statistics module
		statistics.addVariable(Variable("price_estimate", &(currentEstimation.priceChangeEstimate), "Future-aware estimation of efficiency of trades."));

		decltype(requiredSecondaryPairs) allPairs(requiredSecondaryPairs.begin(), requiredSecondaryPairs.end());
		allPairs.push_back("EURUSD");

		for (const std::string & pair : allPairs)
		{
			statistics.addVariable(Variable(pair, std::bind(&VirtualMarket::getLastKnownPrice, this, pair), "Current price of " + pair + "."));
		}

		prepareDayData();

		std::cout << "------------VIRTUAL MARKET SETUP--------(" << config.fromHour << "-" << config.toHour << ")" << std::endl;
		std::cout << "\tTOTAL TICK COUNT\t" << tradingDay->ticks.size() << std::endl;
		std::cout << "\tSTARTING AT TICK\t" << tickIndex << std::endl;
		std::cout << "\tFIRST TICK AT\t" << timeToString(tradingDay->ticks.front().getTime()) << std::endl;
		std::cout << "\tLAST TICK AT\t" << timeToString(tradingDay->ticks.back().getTime()) << std::endl;
		std::cout << "----------------------------------------------" << std::endl;
	}

	VirtualMarket::VirtualMarket()
	{
		tradingDay = nullptr;
		tradeCounter = 1000;
		lastTick = nullptr;
		results.totalProfitPips = 0.0;
		results.wonTrades = results.lostTrades = 0;
		config.fromHour = config.toHour = 0;
		profiler = nullptr;
	}


	VirtualMarket::~VirtualMarket()
	{
		cleanUpDayData();
		delete profiler;
	}


	void VirtualMarket::evaluateDay()
	{
		// act as if remaining trades would have been closed
		for (const auto & trade : trades)
			evaluateTrade(trade, true);
		// save all trade data to file
		saveTrades();
	}

	void VirtualMarket::advanceDay(bool noevaluation)
	{
		// clean up remaining trades and possibly do some statistics
		if (!noevaluation) evaluateDay();
		// advance day and load new data.
		config.date += QuantLib::Period(1, QuantLib::Days);
		prepareDayData();
	}

	void VirtualMarket::cleanUpDayData()
	{
		// clean up old stuff
		delete tradingDay;
		for (auto &day : secondaryCurrencies)
		{
			delete day;
		}
		secondaryCurrencies.clear();
		secondaryCurrenciesIterators.clear();

		trades.clear();
		tradesMetaInfo.clear();
	}

	void VirtualMarket::prepareDayData()
	{
		cleanUpDayData();

		// load tick data
		tradingDay = new TradingDay(config.date, market.getStock("EURUSD", true));
		tradingDay->loadFromFile();
		
		auto checkValid = [](TradingDay *tradingDay)
		{
			return tradingDay->ticks.size() > 15000;
		};

		auto skipDay = [&](int tickCount, std::string currencyPair)
		{
			std::cout << "WARNING: " << currencyPair << " day " << config.date << " has not enough ticks: " << tickCount << std::endl;
			if (config.date <= config.datePeriodEnd)
			{
				advanceDay(true);
				prepareDayData();
			}
			return;
		};

		// check if the day is even OK - arbitrary required tick count here
		if (!checkValid(tradingDay))
		{
			return skipDay(tradingDay->ticks.size(), tradingDay->getCurrencyPair());
		}

		tickIndex = 0;

		
		for (std::string &pair : requiredSecondaryPairs)
		{
			TradingDay *secondaryDay = new TradingDay(config.date, market.getStock(pair, true));
			secondaryCurrencies.push_back(secondaryDay);
			secondaryDay->loadFromFile();

			if (!checkValid(secondaryDay))
				return skipDay(secondaryDay->ticks.size(), secondaryDay->getCurrencyPair());
		}

		// fast forward to start of market day
		if (config.fromHour > 0)
		{
			for (; tickIndex < tradingDay->ticks.size(); ++tickIndex)
			{
				Tick &tick = tradingDay->getTickByIndex(tickIndex);
				std::time_t time = tick.getTime();
				std::tm *tm = std::gmtime(&time);
				if (tm->tm_hour < config.fromHour)
				{
					lastTick = &tick;
					continue;
				}
				break;
			}
		}
	}

	double VirtualMarket::getLastKnownPrice(std::string currencyPair)
	{
		decltype(lastKnownPrice)::iterator iter = lastKnownPrice.find(currencyPair);
		if (iter == lastKnownPrice.end()) return std::numeric_limits<double>::quiet_NaN();
		return iter->second;
	}

	bool VirtualMarket::isOutOfPeriod()
	{
		return config.date > config.datePeriodEnd;
	}

	void VirtualMarket::execute()
	{
		// Market is stop when no more ticks are available.
		// This check is done in the beginning of the function, because at this point the market has processed any prior information.
		if (tickIndex >= tradingDay->ticks.size())
		{
			advanceDay();

			if (isOutOfPeriod())
			{
				std::cout << "VIRTUAL MARKET IS DONE." << std::endl;
				std::cout << "YOUR PROFIT:\t\t" << results.totalProfitPips << std::endl;

				statistics.close();

				if (config.waitOnFinished)
				{
					getchar();
				}
				running = false;
				return;
			}
		}

		// calculate predictive value of efficiency of trades executed at this time point
		predictTradeEfficiency();

		// send the general state of the market BEFORE the ticks
		publishGeneralInfo();

		// send next tick
		std::time_t previousTime = 0;
		if (lastTick) previousTime = lastTick->getTime();

		lastTick = &tradingDay->getTickByIndex(tickIndex++);
		sendTickMsg(lastTick, tradingDay);
		
		// send all ticks of secondary currencies up to the time of the leading currency
		if (secondaryCurrenciesIterators.empty())
		{
			for (TradingDay *&day : secondaryCurrencies)
			{
				secondaryCurrenciesIterators.push_back(day->ticks.begin());
			}
		}
		
		for (size_t secondaryIndex = 0; secondaryIndex < secondaryCurrencies.size(); ++secondaryIndex)
		{
			TradingDay *&day = secondaryCurrencies[secondaryIndex];
			std::vector<Tick>::iterator &iter = secondaryCurrenciesIterators[secondaryIndex];
			while (iter != day->ticks.end())
			{
				const Tick &tick = *iter;
				// fast forward?
				if (tick.getTime() <= previousTime)
				{
					++iter;
					continue;
				}
				// too far..?
				if (tick.getTime() > lastTick->getTime()) break;
				assert(std::isnormal(tick.getAsk()));
				assert(std::isnormal(tick.getBid()));
				sendTickMsg(&tick, day);
				++iter;
			}
		}

		// check end of market day
		if ((config.toHour > 0) && (previousTime != 0))
		{
			std::tm *tm = std::gmtime(&previousTime);
			printf("\r\tTIME %d-%02d-%02d %02d:%02d\tTRADES: %02d\tPROFIT: %.0f", config.date.year(), static_cast<int>(config.date.month()), config.date.dayOfMonth(), tm->tm_hour, tm->tm_min, trades.size(), results.totalProfitPips);
			if (tm->tm_hour > config.toHour)
			{
				// fast forward to end
				tickIndex = tradingDay->ticks.size();
			}
		}
	}

	void VirtualMarket::publishGeneralInfo()
	{
		// account info message gets abused for the profit stuff
		market.account.update(0.0, results.totalProfitPips, results.wonTrades, results.lostTrades);

		// Clear all current trades in the market (same would be done by a conventional update message)
		market.saveAndClearTrades();
		// publish all currently open trades 
		for (Trade &trade : trades)
		{
			// do not simply copy the trade to mimick the interface as closely as possible...
			Trade *newTrade = new Trade();
			newTrade->currencyPair = trade.currencyPair;
			newTrade->lotSize = trade.lotSize;
			newTrade->orderPrice = trade.orderPrice;
			newTrade->setStopLossPrice(trade.getStopLossPrice());
			newTrade->setTakeProfitPrice(trade.getTakeProfitPrice());
			newTrade->ticketID = trade.ticketID;
			newTrade->type = trade.type;

			market.onNewTradeMessageReceived(newTrade);
		}
	}

	void VirtualMarket::sendTickMsg(const Tick *tick, TradingDay *day)
	{
		lastKnownPrice[day->getCurrencyPair()] = tick->getMid();
		market.onNewTickMessageReceived(day->getCurrencyPair(), tick->getBid(), tick->getAsk(), tick->getTime());
	}

	void VirtualMarket::onReceive(const std::string &message)
	{
		assert(message[0] == 'C');

		std::istringstream is(message);
		std::string accountInfo, command;
		is >> accountInfo >> accountInfo >> command;

		if (command == "set")
		{
			int tradeType;
			std::string pair;
			Trade trade;
			is >> tradeType >> trade.currencyPair >> trade.orderPrice >> trade.getTakeProfitPrice() >> trade.getStopLossPrice() >> trade.lotSize;
			
			trade.type = (tradeType == 0) ? Trade::T_BUY : Trade::T_SELL;
			trade.ticketID = ++tradeCounter;
			trade.removeSaveFile();
			trades.push_back(trade);

			tradesMetaInfo.emplace(trade.ticketID, VirtualTradeMetaInfo(trade.type, market.getLastTickTime()));
		}
		else if (command == "unset")
		{
			int ticketID;
			is >> ticketID;
			
			for (auto iter = trades.begin(); iter != trades.end(); ++iter)
			{
				Trade &trade = *iter;
				if (trade.ticketID != ticketID) continue;
				evaluateTrade(trade);
				trade.removeSaveFile();
				trades.erase(iter);
				break;
			}
		}
	}

	void VirtualMarket::evaluateTrade(const Trade &trade, bool forceful)
	{
		if (trade.currencyPair != tradingDay->getCurrencyPair()) return;

		QuantLib::Decimal profit = trade.getProfitAtTick(*lastTick);
		profit /= ONEPIP;
		
		if (profit > 0.0) results.wonTrades += 1;
		else if (profit < 0.0) results.lostTrades += 1;
		results.totalProfitPips += profit;

		tradesMetaInfo.at(trade.ticketID).setClosed(profit, market.getLastTickTime(), lastTick->getTime(), forceful);
	}

	void VirtualMarket::proxySend(const std::string &message)
	{
		pendingMessages.push(message);
	}

	std::string VirtualMarket::proxyReceive()
	{
		if (pendingMessages.empty()) return "";
		std::string value = pendingMessages.front();
		pendingMessages.pop();
		return value;
	}

	void VirtualMarket::predictTradeEfficiency()
	{
		if (!lastTick) return;

		std::time_t time = lastTick->getTime();

		const int lookaheadTime = 15 * ONEMINUTE;
		std::time_t endTime = time + lookaheadTime;
		if (endTime > tradingDay->ticks.back().getTime()) return;

		TimePeriod period = TimePeriod(nullptr, time, endTime, &Tick::getMid);
		period.setTradingDay(tradingDay);
		const std::vector<QuantLib::Decimal> price = period.toVector(ONEMINUTE);

		auto assesPrice = [&]()
		{
			QuantLib::Decimal min = 0.0;
			QuantLib::Decimal max = 0.0;
			int lastSign = 0;
			for (size_t i = 1; i < price.size(); ++i)
			{
				QuantLib::Decimal currentChange = price[i] - price[0];
				
				int sign = Math::signum(currentChange);

				if (lastSign != 0 && sign != lastSign) break;
				lastSign = sign;

				if (currentChange > max) max = currentChange;
				if (currentChange < min) min = currentChange;
			}
			assert(min == 0.0 || max == 0.0);
			decltype(min) value = 0.0;
		
			if (std::abs(min) > std::abs(max)) value = min;
			else value = max;
			return value / ONEPIP;
		};

		PossibleDecimal open;
		open = period.getOpen();
		if (!open) return;

		currentEstimation.priceChangeEstimate = assesPrice();
	}

	void VirtualMarket::saveTrades()
	{
		std::ios_base::openmode mode = std::ios_base::out;
		if (results.tradesHeaderPrinted)
			mode |= std::ios_base::app;
		else
			mode |= std::ios_base::trunc;
		std::fstream output("saves/trades.tsv", mode);
		// headlines only when exporting the trades for the very first day
		if (!results.tradesHeaderPrinted)
			output << "Trade ID\tOpening Time\tType\tProfit\tClosing Time\tForced Close" << std::endl;

		for (auto &data : tradesMetaInfo)
		{
			int32_t ID = data.first;
			VirtualTradeMetaInfo &metaData = data.second;

			output
				<< ID << "\t"
				<< metaData.openingTime << "\t"
				<< metaData.type << "\t"
				<< metaData.profit
				<< "\t" << metaData.closingTime
				<< "\t" << metaData.forcefulClose
				<< std::endl;
		}

		results.tradesHeaderPrinted = true;
	}
};
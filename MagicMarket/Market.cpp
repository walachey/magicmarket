#include "Market.h"

#include <sstream>
#include <iostream>
#include <ctime>
#include <thread>
#include <iomanip>

#include <filesystem>
namespace filesystem = std::tr2::sys;

#include <SimpleIni.h>

#include "Interfaces/MTInterface.h"
#include "VirtualMarket.h"
#include "Statistics.h"

#include "Stock.h"
#include "Trade.h"
#include "TradingDay.h"
#include "ExpertAdvisor.h"
#include "ExpertAdvisorExternal.h"
#include "ExpertAdvisorLimitAdjuster.h"
#include "ExpertAdvisorRSI.h"
#include "ExpertAdvisorCCI.h"
#include "ExpertAdvisorTSI.h"
#include "ExpertAdvisorStochasticOscillator.h"
#include "ExpertAdvisorRenko.h"
#include "ExpertAdvisorBroker.h"
#include "ExpertAdvisorDumbo.h"
#include "ExpertAdvisorMultiCurrency.h"
#include "ExpertAdvisorAtama.h"
#include "ExpertAdvisorMAAnalyser.h"

#include "Indicators/LocalRelativeChange.h"
#include "Indicators/TargetLookbackMean.h"
#include "Indicators/ADX.h"

#include "thirdparty/json11.hpp"

#include "DeepLearningTest.h"

MM::Market market;

namespace MM
{

	Market::Market()
	{
		isVirtualModeEnabled = false;
		lastTickTime = 0;
		lastTickDate = QuantLib::Date();
	}


	Market::~Market()
	{
		for (Indicators::Base *&indicator : indicators)
		{
			delete indicator;
		}
		indicators.clear();

		for (ExpertAdvisor *&expert : experts)
		{
			delete expert;
		}
		experts.clear();
	}

	void Market::addEvent(const Event &e)
	{
		// todo: check logic? Shouldn't only same-type events be replaced?
		for (size_t i = 0, ii = events.size(); i < ii; ++i)
		{
			if (events[i] == e) return;
			if (events[i].youngerThan(e))
			{
				events[i] = e;
				return;
			}
			if (e.youngerThan(events[i])) return;
		}
		events.push_back(e);
	}

	void Market::init(void *_ini)
	{
		const CSimpleIniA &ini = *(CSimpleIniA*)_ini;
		int sleepDurationMs;
		std::istringstream(ini.GetValue("Market", "SleepDuration", "100")) >> sleepDurationMs;
		sleepDuration = std::chrono::milliseconds(sleepDurationMs);
		std::cout << "..loaded config" << std::endl;

		tradingConfiguration.initialStopLoss = ini.GetDoubleValue("Market", "InitialStopLoss", 0.0);

		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRSI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorCCI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorTSI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorStochasticOscillator()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRenko(5.0)));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRenko(20.0)));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorDumbo()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorMultiCurrency()));
		//experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorMAAnalyser()));
		//experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorAtama()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorLimitAdjuster()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorBroker()));

		// Enforce some more indicators to exist.
		Indicators::get<Indicators::ADX>("EURUSD", 20, 2 * ONEMINUTE);
		Indicators::get<Indicators::TargetLookbackMean>("EURUSD", 15);

		{ // scope
			const std::vector<int> lookbackDurations =
			{
				60 * ONEMINUTE,
				30 * ONEMINUTE,
				15 * ONEMINUTE,
				10 * ONEMINUTE,
				5 * ONEMINUTE,
				2 * ONEMINUTE,
				1 * ONEMINUTE,
				30 * ONESECOND,
				0
			};
			for (const std::string &currencyPair : { "EURUSD", "EURCHF", "EURGBP", "GBPUSD", "USDCHF", "USDJPY", "EURUSD" })
				Indicators::get<Indicators::LocalRelativeChange>(currencyPair, lookbackDurations);
		}

		// And now the external interfaces.
		for (int i = 0; i < 255; ++i)
		{
			const std::string configName = std::string("External Agent ") + std::to_string(i + 1);
			std::string endpoint = ini.GetValue(configName.c_str(), "Endpoint", "");
			if (endpoint.empty()) break;

			auto externalAgent = new ExpertAdvisorExternal();
			bool connected = externalAgent->connect(endpoint);

			if (connected)
				experts.push_back(static_cast<ExpertAdvisor*>(externalAgent));
			else
				delete externalAgent;
		}

		// Now resort experts to make sure dependencies are evaluated in the correct order.
		{ // scope
			std::list<ExpertAdvisor*> expertsUnsorted(experts.begin(), experts.end());
			experts.clear();
			experts.reserve(expertsUnsorted.size());
			// Prepare some place for experts that necessarily come last. (e.g.: the broker)
			std::list<std::pair<int, ExpertAdvisor*>> finalExperts;
			auto addFinalExpert = [&finalExperts](ExpertAdvisor *expert, int priority)
			{
				auto iter = finalExperts.begin();
				for (; iter != finalExperts.end(); ++iter)
				{
					const int &currentPriority = iter->first;
					if (currentPriority <= priority) continue;
					break;
				}
				finalExperts.insert(iter, std::make_pair(priority, expert));
			};

			while (!expertsUnsorted.empty())
			{
				bool addedOne = false;

				for (auto iter = expertsUnsorted.begin(); iter != expertsUnsorted.end();)
				{
					ExpertAdvisor *expert = *iter;
					const std::vector<std::string> dependencies = expert->getRequiredExperts();

					// Trivial dependencies?
					if (dependencies.empty())
					{
						experts.push_back(expert);
						iter = expertsUnsorted.erase(iter);
						addedOne = true;
						continue;
					}

					// Normal dependencies.
					bool dependenciesSatisfied = true;
					int requireAllPriority = 0;
					for (std::string const & dep : dependencies)
					{
						// Needs all others? (e.g. the broker)
						if (dep[0] == '*')
						{
							requireAllPriority = dep.size();
							continue;
						}
						// Check available experts for name matches.
						bool found = false;
						for (ExpertAdvisor * availableExpert : experts)
						{
							if (availableExpert->getName() != dep) continue;
							found = true;
							break;
						}
						if (!found) dependenciesSatisfied = false;
					}
					// If an expert needs all others, the condition is different.
					if (requireAllPriority > 0)
					{
						addFinalExpert(expert, requireAllPriority);
						iter = expertsUnsorted.erase(iter);
						addedOne = true;
						continue;
					}

					if (dependenciesSatisfied)
					{
						experts.push_back(expert);
						iter = expertsUnsorted.erase(iter);
						addedOne = true;
						continue;
					}

					// Skip this one and try the next.
					++iter;
				}

				// Failsafe.
				if (!addedOne)
				{
					std::cerr << "Could not satisfy dependencies of following experts:" << std::endl;
					for (ExpertAdvisor * expert : expertsUnsorted)
					{
						std::string dependencies = "";
						for (std::string const & dep : expert->getRequiredExperts())
							dependencies += (dependencies.empty() ? "" : ", ") + dep;
						std::cerr << "\t- " << expert->getName() << " (depends on " << dependencies << ")" << std::endl;
					}
					break;
				}
			}
			// Now add the final experts.
			for (auto &finalExpert : finalExperts)
				experts.push_back(finalExpert.second);
		} // Expert dependency resolving.

		for (ExpertAdvisor * const & indicator : indicators)
		{
			indicator->declareExports();
			indicator->onNewDay();
		}
		for (ExpertAdvisor * const & expert : experts)
		{
			expert->declareExports();
		}
		for (ExpertAdvisor * const & expert : experts)
		{
			expert->afterExportsDeclared();
			expert->onNewDay();
		}

		// Now print the experts to console.
		std::ostringstream expertInformation;
		expertInformation << "Available Indicators: ";
		for (const ExpertAdvisor * indicator : indicators)
			expertInformation << indicator->getName() << (indicator == indicators.back()) ? "" : ", ";
		expertInformation << "\nAvailable Experts: ";
		for (const ExpertAdvisor * expert: experts)
			expertInformation << expert->getName() << (expert == experts.back()) ? "" : ", ";
	}

	void Market::run()
	{
		// for the experts' execute() callback
		std::time_t startTime = (0), lastExecutionTime(0);

		// for debugging & testing
		bool onlyOnce = false;
		
		while (true)
		{
			if (!isVirtual())
				metatrader.checkIncomingMessages();
			
			if (!onlyOnce)
			{
				//newTrade(MM::Trade::Sell("EURUSD", 0.01));
				onlyOnce = true;
				/*
				DeepLearningTest test;
				test.run();
				exit(1);
				*/
			}

			// if new things happened, notify the experts
			for (Event &event : events)
			{
				lastTickTime = std::max(lastTickTime, event.time);
				/*
					Quickly reset experts when event for new day occurs.
					This is mainly important for the virtual market but it should be done here;
					this decreases the divergence between the virtual market execution and the real one.
					Additionally it increases the probability of finding reset-related bugs early on.
				*/
				const QuantLib::Date &date = event.date;
				if (date != lastTickDate)
				{
					lastTickDate = date;

					for (ExpertAdvisor *&expert : experts)
						expert->onNewDay();
					for (Indicators::Base *&indicator : indicators)
						indicator->onNewDay();
				}

				switch (event.type)
				{
				case Event::Type::NEW_TICK:
					for (ExpertAdvisor *&expert : experts)
					{
						expert->onNewTick(event.currencyPair, event.date, event.time);
					}

					break;
				case Event::Type::TIMER:
					break;
				default:
					assert(false);
					break;
				}
			}
			events.clear();

			// allow all experts to execute if they want to
			if (startTime == 0) startTime = lastTickTime;
			std::time_t timePassed = lastTickTime - startTime;
			if ((timePassed != lastExecutionTime) && (lastTickTime != 0))
			{
				lastExecutionTime = timePassed;

				// update indicators first
				for (Indicators::Base *&indicator : indicators)
				{
					indicator->execute(timePassed, lastTickTime);
				}

				for (ExpertAdvisor *&expert : experts)
				{
					expert->execute(timePassed, lastTickTime);
				}

				statistics.log();
			}
			if (isVirtual())
			{
				virtualMarket->execute();
				if (!virtualMarket->isRunning()) return;
			}
			else
			{
				if (trades.empty())
				{
					std::cout << "\rNo open positions." << std::flush;
				}
				else
				{
					system("cls");
					std::cout << "|TRADES\t|\tProfit\t|\tS/L\t|\t|" << std::endl;
					std::cout << "---------------------------------------------------------" << std::endl;
					double totalProfit = 0.0;
					const int numberOfTrades = trades.size();
					for (Trade * trade : trades)
					{
						Stock *stock = market.getStock(trade->currencyPair);
						TimePeriod period = stock->getTimePeriod(lastTickTime);
						const Tick *lastTick = period.getLastTick();
						const QuantLib::Decimal currentProfit = (lastTick != nullptr) ? trade->getProfitAtTick(*lastTick) : std::numeric_limits<double>::quiet_NaN();
						totalProfit += currentProfit;

						std::cout << "|" << (trade->type == Trade::Type::T_BUY ? "BUY" : "SELL") << "\t|\t"
							<< (currentProfit / ONEPIP) << "\t|\t" << trade->getStopLossPrice() << "\t|" << std::endl;

					}
					std::cout << "---------------------------------------------------------" << std::endl;
					std::cout << "\rTRADES: " << numberOfTrades << "\tPROFIT: " << (totalProfit / ONEPIP) << " pips" << std::flush;
				}
				if (sleepDuration > std::chrono::milliseconds(0))
					std::this_thread::sleep_for(sleepDuration);
			}
		}
	}

	Trade *Market::newTrade(Trade trade)
	{
		Trade *accepted = new Trade(trade);

		// allow all experts to fool with the trade
		for (ExpertAdvisor *&expert : experts)
		{
			if (expert->acceptNewTrade(accepted)) continue;
			delete accepted;
			return nullptr;
		}

		//trades.push_back(accepted);

		int tradeType = -1;

		if (trade.type == Trade::T_BUY)
			tradeType = 0;
		else if (trade.type == Trade::T_SELL)
			tradeType = 1;
		assert(tradeType != -1);


		::Interface::MetaTrader::Message::NewOrder message;
		message.type = tradeType;
		message.orderPrice = accepted->orderPrice;
		message.takeProfitPrice = accepted->getTakeProfitPrice();
		message.stopLossPrice = accepted->getStopLossPrice();
		message.lotSize = accepted->lotSize;
		
		if (market.isVirtual()) virtualMarket->onReceive(message);
		else metatrader.send(message);

		assert(trades.size() < 10);
		return nullptr;
	}

	void Market::updateTrade(Trade *trade)
	{
		assert(trade->ticketID != -1);

		::Interface::MetaTrader::Message::UpdateOrder message;

		message.ticketID = trade->ticketID;
		message.takeProfitPrice = trade->getTakeProfitPrice();
		message.stopLossPrice = trade->getStopLossPrice();
		
		if (market.isVirtual()) virtualMarket->onReceive(message);
		else metatrader.send(message);
	}

	void Market::closeTrade(Trade *trade)
	{
		assert(trade->ticketID != -1);

		::Interface::MetaTrader::Message::CloseOrder message;

		message.ticketID = trade->ticketID;

		if (market.isVirtual()) virtualMarket->onReceive(message);
		else metatrader.send(message);
	}

	void Market::chat(std::string name, std::string msg)
	{
		std::ostringstream os;
		os << "! " << name
			<< " " << getLastTickTime()
			<< " " << msg;
		// send(os.str());
	}

	void Market::onNewTickMessageReceived(const std::string &pair, QuantLib::Decimal bid, QuantLib::Decimal ask, std::time_t time)
	{
		Tick tick;
		tick.bid = bid;
		tick.ask = ask;
		tick.time = time;

		getStock(pair, true)->receiveFreshTick(tick);
		// notify the experts asap
		addEvent(Event(Event::Type::NEW_TICK, pair, QuantLib::Date::todaysDate(), tick.time));
	}

	void Market::onNewTradeMessageReceived(Trade *trade)
	{
		trade->load();
		trades.push_back(trade);
	}

	// When updating the trades either trough the virtual market or by a new comprehensive update orders message.
	void Market::saveAndClearTrades()
	{
		// remove all (non virtual) trades
		for (size_t i = 0; i < trades.size(); ++i)
		{
			Trade *& trade = trades[i];
			// if (trade.isVirtual()) continue;
			trade->save();
			delete trade;
			trades[i] = nullptr;
		}
		trades.erase(std::remove(std::begin(trades), std::end(trades), nullptr), std::end(trades));
		assert(trades.size() == 0); // for now
	}

	void Market::addStock(std::string pair)
	{
		if (stocks.count(pair)) return;
		stocks[pair] = new Stock(pair);
	}

	Stock* Market::getStock(std::string pair, bool allowCreation)
	{
		if (stocks.count(pair)) return stocks[pair];

		std::string pathString = Stock::getDirectoryName(pair);
		filesystem::path path(pathString);

		if (!filesystem::exists(path) && !allowCreation) return nullptr;

		// create it, it will load its stuff lazily
		Stock *stock = new Stock(pair);
		stocks[pair] = stock;
		return stock;
	}

	void Market::setSleepDuration(int ms)
	{
		sleepDuration = std::chrono::milliseconds(ms);
	}
};
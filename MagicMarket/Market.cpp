#include "Market.h"

#include <sstream>
#include <iostream>
#include <ctime>
#include <thread>
#include <iomanip>

#include <filesystem>
namespace filesystem = std::tr2::sys;

#include <SimpleIni.h>

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
#include "ExpertAdvisorAtama.h"
#include "ExpertAdvisorMAAnalyser.h"

#include "Indicators/LocalRelativeChange.h"

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

	void Market::setupConnection()
	{
		int errorValue;
		zmqContext = zmq_init(1);
		assert(zmqContext);
		zmqListener = zmq_socket(zmqContext, ZMQ_SUB);
		assert(zmqListener);
		errorValue = zmq_setsockopt(zmqListener, ZMQ_SUBSCRIBE, "", 0);
		assert(errorValue == 0);
		errorValue = zmq_connect(zmqListener, connectionStringListener.c_str());
		assert(errorValue == 0);

		zmqPublisher = zmq_socket(zmqContext, ZMQ_PUB);
		assert(zmqPublisher);
		errorValue = zmq_connect(zmqPublisher, connectionStringSpeaker.c_str());
		assert(errorValue == 0);

		std::cout << "Listener: \t" << connectionStringListener << std::endl;
		std::cout << "Speaker: \t" << connectionStringSpeaker << std::endl;
		std::cout << "..connected" << std::endl;
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
		uid = ini.GetValue("Metatrader", "UserID", "unknown");
		accountName = ini.GetValue("Metatrader", "AccountName", "unknown");
		connectionStringListener = ini.GetValue("Central Station", "Listener", "tcp://127.0.0.1:1985");
		connectionStringSpeaker = ini.GetValue("Central Station", "Speaker", "tcp://127.0.0.1:1986");
		int sleepDurationMs;
		std::istringstream(ini.GetValue("Market", "SleepDuration", "100")) >> sleepDurationMs;
		sleepDuration = std::chrono::milliseconds(sleepDurationMs);
		std::cout << "..loaded config" << std::endl;

		setupConnection();

		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRSI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorCCI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorTSI()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorStochasticOscillator()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRenko(5.0)));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorRenko(10.0)));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorDumbo()));
		//experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorMAAnalyser()));
		//experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorAtama()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorLimitAdjuster()));
		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorBroker()));

		// Enforce some more indicators to exist.
		{ // scope
			const std::vector<int> lookbackDurations =
			{
				30 * ONEMINUTE,
				15 * ONEMINUTE,
				10 * ONEMINUTE,
				5 * ONEMINUTE,
				2 * ONEMINUTE,
				1 * ONEMINUTE,
				30 * ONESECOND
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
	}

	void Market::run()
	{
		// for the experts' execute() callback
		std::time_t startTime = (0), lastExecutionTime(0);

		// for debugging & testing
		bool onlyOnce = false;
		
		while (true)
		{
			// receive new data over the ZMQ interface
			std::string data;
			do
			{
				if (isVirtual())
					data = virtualMarket->proxyReceive();
				else
					data = receive();

				if (data.size())
				{
					parseMessage(data);
				}
			} while (data.size());
			
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
			
			if (sleepDuration > std::chrono::milliseconds(0))
				std::this_thread::sleep_for(sleepDuration);
		}
	}

	std::string Market::getCommandPrefix()
	{
		return std::string("C ") + accountName + "|" + uid;
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

		std::ostringstream os;
		os << getCommandPrefix() << 
			" set " << tradeType << 
			" " << accepted->currencyPair << 
			" " << accepted->orderPrice << 
			" " << accepted->getTakeProfitPrice() << 
			" " << accepted->getStopLossPrice() << 
			" " << accepted->lotSize;
		send(os.str());
		assert(trades.size() < 10);
		return nullptr;
	}

	void Market::updateTrade(Trade *trade)
	{
		assert(trade->ticketID != -1);

		std::ostringstream os;
		os << getCommandPrefix() <<
			" reset " << trade->ticketID <<
			" " << trade->getTakeProfitPrice() <<
			" " << trade->getStopLossPrice();
		send(os.str());
	}

	void Market::closeTrade(Trade *trade)
	{
		assert(trade->ticketID != -1);

		std::ostringstream os;
		os << getCommandPrefix() <<
			" unset " << trade->ticketID;
		send(os.str());
	}

	void Market::updateMood(std::string name, float mood, float certainty)
	{
		std::ostringstream os; os << "M " << name << " " << mood << " " << certainty;
		send(os.str());
	}

	void Market::updateParameter(std::string name, double value)
	{
		std::ostringstream os; os << "P " << name << " " << std::setprecision(3) << value;
		send(os.str());
	}

	void Market::chat(std::string name, std::string msg)
	{
		std::ostringstream os;
		os << "! " << name
			<< " " << getLastTickTime()
			<< " " << msg;
		send(os.str());
	}

	std::string Market::receive()
	{
		int errorValue;
		zmq_msg_buf msg;

		errorValue = zmq_msg_recv(&msg.msg, zmqListener, ZMQ_DONTWAIT);
		if (errorValue == -1 && errno == EAGAIN) return "";
		assert(errorValue >= 0);

		// we have a 
		std::string data(static_cast<char*>(zmq_msg_data(&msg.msg)), zmq_msg_size(&msg.msg));
		assert(data.size() == errorValue);

		// check if more!
		int32_t more;
		size_t more_size = sizeof(more);
		do
		{
			msg = zmq_msg_buf();
			errorValue = zmq_getsockopt(zmqListener, ZMQ_RCVMORE, &more, &more_size);
			assert(errorValue == 0);
			if (!more) break;

			errorValue = zmq_msg_recv(&msg.msg, zmqListener, ZMQ_DONTWAIT);
			if (errorValue == -1)
			{
				std::cerr << "ZMQ ERROR (recv): " << zmq_strerror(errno) << std::endl;
			}
			assert(errorValue >= 0);
			data.append(static_cast<char*>(zmq_msg_data(&msg.msg)), zmq_msg_size(&msg.msg));
		} while (more);
		return data;
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

	void Market::parseMessage(const std::string &message)
	{
		//std::cout << "received:\n\t" << message << std::endl;
		if (message.empty()) return;

		std::istringstream is(message);
		char type;
		std::string accountName;

		is >> type >> accountName;

		if (type == 'T')
		{
			std::string pair;
			QuantLib::Decimal bid, ask;
			std::time_t time;
			is >> pair >> bid >> ask >> time;
			onNewTickMessageReceived(pair, bid, ask, time);
		}
		else if (type == 'R')
		{
			std::string uid;
			is >> uid;
		}
		else if (type == 'O')
		{
			saveAndClearTrades();


			// dummy trade
			/*Trade *trade = new Trade();
			trade->currencyPair = "EURUSD";
			trade->lotSize = 10.;
			trade->orderPrice = 0.5;
			trade->stopLossPrice = 0.0;
			trade->takeProfitPrice = 0.0;
			trade->ticketID = 213214;
			trades.push_back(trade);
			return;*/

			// and fill with current trade data
			std::string jsonString;
			getline(is, jsonString);
			jsonString.erase(0, 1);

			// I don't know yet why this is necessary, TODO
			if (jsonString[jsonString.length()-1] == '\0') // seems to happen only in virtual mode on debug
				jsonString = jsonString.substr(0, jsonString.size() - 1);

			std::string errorString;
			json11::Json jsonData = json11::Json::parse(jsonString, errorString);

			for (auto &order : jsonData.array_items())
			{
				Trade *trade = new Trade();
				trade->currencyPair = order["pair"].string_value();
				trade->lotSize = order["lots"].number_value();
				trade->orderPrice = order["open_price"].number_value();
				trade->setStopLossPrice(order["stop_loss"].number_value());
				trade->setTakeProfitPrice(order["take_profit"].number_value());
				trade->ticketID = order["ticket_id"].int_value();

				if (order["type"].int_value() == 0) trade->type = Trade::T_BUY;
				else trade->type = Trade::T_SELL;
				
				onNewTradeMessageReceived(trade);
			}

		}
		else if (type == 'A')
		{
			QuantLib::Decimal leverage, balance, margin, marginFree;
			is >> leverage
				>> balance
				>> margin
				>> marginFree;
			account.update(leverage, balance, margin, marginFree);
		}
		else if (isVirtual() && (type == 'C'))
		{
			virtualMarket->onReceive(message);
		}
		else
		{
			// pass
		}
	}

	void Market::send(std::string data, int probabilityToSendInVirtualMode)
	{
		if (isVirtual())
		{
			virtualMarket->proxySend(data);
			if (probabilityToSendInVirtualMode == 0) return;
			if (probabilityToSendInVirtualMode < 100)
			{
				int randomDraw = rand() % 100;
				if (randomDraw > probabilityToSendInVirtualMode) return;
			}

			if (virtualMarket->isSilent) return;
		}

		zmq_msg_t msg;
		zmq_msg_init_size(&msg, data.length() + 1);
		memcpy(zmq_msg_data(&msg), data.c_str(), data.length() + 1);

		int errorValue;
		errorValue = zmq_msg_send(&msg, zmqPublisher, 0);

		if (errorValue == -1 && errno == EAGAIN)
			std::cerr << "send currently not possible." << std::endl; // OK
		else
		{
			if (errorValue < 0)
			{
				std::cerr << "ZMQ ERROR (send): " << zmq_strerror(errno) << std::endl;
			}
			assert(errorValue >= 0);
		}
		// std::cout << "SENT:\n\t" << data << std::endl;

		zmq_msg_close(&msg);
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
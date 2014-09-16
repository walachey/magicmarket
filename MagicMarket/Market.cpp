#include "Market.h"

#include <sstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>

#include <filesystem>
namespace filesystem = std::tr2::sys;

#include <SimpleIni.h>


#include "Stock.h"
#include "Trade.h"
#include "TradingDay.h"
#include "ExpertAdvisor.h"
#include "ExpertAdvisorLimitAdjuster.h"

#include "thirdparty/json11.hpp"

MM::Market market;

namespace MM
{

	Market::Market()
	{
	}


	Market::~Market()
	{
	}

	void Market::loadConfig()
	{
		CSimpleIniA ini;
		ini.LoadFile("market.ini");

		if (ini.IsEmpty())
		{
			std::cout << "ERROR: Could not load configuration!" << std::endl;
		}

		uid = ini.GetValue("Metatrader", "UserID", "unknown");
		accountName = ini.GetValue("Metatrader", "AccountName", "unknown");
		connectionStringListener = ini.GetValue("Central Station", "Listener", "tcp://127.0.0.1:1985");
		connectionStringSpeaker = ini.GetValue("Central Station", "Speaker", "tcp://127.0.0.1:1986");
		std::cout << "..loaded config" << std::endl;
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

	void Market::init()
	{
		loadConfig();
		setupConnection();

		experts.push_back(static_cast<ExpertAdvisor*>(new ExpertAdvisorLimitAdjuster()));
	}

	void Market::run()
	{
		// sleep duration between loops
		std::chrono::milliseconds sleepDuration(100);
		// for the experts' execute() callback
		std::time_t startTime = time(0), lastExecutionTime(0);
		// for debugging & testing
		bool onlyOnce = false;
		
		while (true)
		{
			// receive new data over the ZMQ interface
			std::string data;
			do
			{
				data = receive();
				if (data.size())
				{
					parseMessage(data);
				}
			} while (data.size());
			
			if (!onlyOnce)
			{
				//MM::Trade::Buy("EURUSD", 0.01);
				onlyOnce = true;
			}

			// if new things happened, notify the experts
			for (Event &event : events)
			{
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
			std::time_t timePassed = time(0) - startTime;
			if (timePassed != lastExecutionTime)
			{
				lastExecutionTime = timePassed;
				for (ExpertAdvisor *&expert : experts)
				{
					expert->execute(timePassed);
				}
			}
			std::this_thread::sleep_for(sleepDuration);
		}
	}

	std::string Market::getCommandPrefix()
	{
		return std::string("cmd|") + accountName + "|" + uid;
	}

	Trade *Market::newTrade(Trade trade)
	{
		Trade *accepted = new Trade(trade);
		trades.push_back(accepted);

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
			" " << accepted->takeProfitPrice << 
			" " << accepted->stopLossPrice << 
			" " << accepted->lotSize;
		send(os.str());

		return accepted;
	}

	void Market::updateTrade(Trade *trade)
	{
		assert(trade->ticketID != -1);

		std::ostringstream os;
		os << getCommandPrefix() <<
			" reset " << trade->ticketID <<
			" " << trade->takeProfitPrice <<
			" " << trade->stopLossPrice;
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
		std::ostringstream os; os << "mood " << name << " " << mood << " " << certainty;
		send(os.str());
	}

	void Market::chat(std::string name, std::string msg)
	{
		std::ostringstream os;
		os << "chat " << name
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

	void Market::parseMessage(const std::string &message)
	{
		//std::cout << "received:\n\t" << message << std::endl;

		std::istringstream is(message);
		std::string type, accountName;

		is >> type >> accountName;

		if (type == "tick")
		{
			std::string pair;
			Tick tick;
			is >> pair >> tick.bid >> tick.ask >> tick.time;
			getStock(pair, true)->receiveFreshTick(tick);
			//std::cout << "Fresh tick:\n\t" << tick << std::endl;

			// notify the experts asap
			addEvent(Event(Event::Type::NEW_TICK, pair, QuantLib::Date::todaysDate(), tick.time));
		}
		else if (type == "ema")
		{
			
		}
		else if (type == "response")
		{
			std::string uid;
			is >> uid;
		}
		else if (type == "orders")
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
			std::string errorString;
			json11::Json jsonData = json11::Json::parse(jsonString, errorString);

			for (auto &order : jsonData.array_items())
			{
				Trade *trade = new Trade();
				trade->currencyPair = order["pair"].string_value();
				trade->lotSize = order["lots"].number_value();
				trade->orderPrice = order["open_price"].number_value();
				trade->stopLossPrice = order["stop_loss"].number_value();
				trade->takeProfitPrice = order["take_profit"].number_value();
				trade->ticketID = order["ticket_id"].int_value();

				if (order["type"].int_value() == 0) trade->type = Trade::T_BUY;
				else trade->type = Trade::T_SELL;
				
				trade->load();

				trades.push_back(trade);
			}

		}
		else if (type == "account")
		{
			is >> account.leverage
				>> account.balance
				>> account.margin
				>> account.marginFree;
		}

	}

	void Market::send(std::string data)
	{
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
		std::cout << "SENT:\n\t" << data << std::endl;

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
};
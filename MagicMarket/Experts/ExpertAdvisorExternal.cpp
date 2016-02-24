#include "ExpertAdvisorExternal.h"
#include "Market.h"
#include "Statistics.h"
#include "Trade.h"
#include "Stock.h"

#include "Interfaces\Expert.pb.h"

#include <algorithm>

namespace MM
{
	void ExpertAdvisorExternal::reset()
	{
		Interfaces::ExpertMessage message;
		message.set_type(Interfaces::ExpertMessage_Type_reset);
		send(message);
		receive();
	}

	ExpertAdvisorExternal::ExpertAdvisorExternal()
	{
		executive = false;
	}


	ExpertAdvisorExternal::~ExpertAdvisorExternal()
	{
		if (socket.get() != nullptr)
		{
			std::cout << "External Agent: sending shutdown signal to zmq::" << getName() << std::endl;
			Interfaces::ExpertMessage message;
			message.set_type(Interfaces::ExpertMessage_Type_shutdown);
			send(message);
			receive();
		}
	}

	void ExpertAdvisorExternal::send(Interfaces::ExpertMessage &message)
	{
		std::string serialized;
		message.SerializeToString(&serialized);
		socket->send(serialized.data(), serialized.size(), 0);
	}

	Interfaces::ExpertMessage ExpertAdvisorExternal::receive()
	{
		zmq::message_t reply;
		socket->recv(&reply);
		std::string replyString(static_cast<char*>(reply.data()), reply.size());

		Interfaces::ExpertMessage message;
		message.ParseFromString(replyString);
		return message;
	}

	bool ExpertAdvisorExternal::connect(std::string endpoint)
	{
		std::cout << "Connecting to python..." << std::endl;
		// connect to python script..
		context = std::make_unique<zmq::context_t>(1);
		
		socket = std::make_unique<zmq::socket_t>(*context.get(), ZMQ_REQ);
		try
		{
			socket->connect(endpoint.c_str());
		}
		catch (zmq::error_t &error)
		{
			std::cout << "\terror connecting: " << error.what() << std::endl;
			return false;
		}

		{ // Handshake
			Interfaces::ExpertMessage message;
			message.set_type(Interfaces::ExpertMessage_Type::ExpertMessage_Type_getName);
			send(message);

			Interfaces::ExpertMessage reply = receive();
			if (reply.type() == Interfaces::ExpertMessage_Type_getName && reply.has_name())
			{
				name = reply.name();
				std::cout << "\tLinking expert " << name << "..." << std::endl;
			}
			else
			{
				std::cout << "\tReceived invalid get_name reply." << std::endl;
			}
		}

		{ // Agent Informations
			Interfaces::ExpertMessage message;
			message.set_type(Interfaces::ExpertMessage_Type::ExpertMessage_Type_informations);
			send(message);

			Interfaces::ExpertMessage reply;
			reply = receive();
			assert(reply.type() == Interfaces::ExpertMessage_Type_informations);
			executive = reply.information().isexecutive();
			std::cout << "\tExpert role:\t" << (executive ? "executive" : "supportive") << std::endl;
		}

		{ // Required Variables
			Interfaces::ExpertMessage message;
			message.set_type(Interfaces::ExpertMessage_Type_getRequiredVariables);
			send(message);

			Interfaces::ExpertMessage reply = receive();
			assert(reply.type() == Interfaces::ExpertMessage_Type_getRequiredVariables);

			requiredVariables = std::vector<std::string>(reply.variablenames().begin(), reply.variablenames().end());
		}

		std::cout << "\tExpert " << name << " linked." << std::endl;

		return true;
	}

	void ExpertAdvisorExternal::afterExportsDeclared()
	{
		size_t varCount = 0;
		for (const std::string &namedesc : requiredVariables)
		{
			std::istringstream os(namedesc);
			std::string name, desc;
			os >> name;
			std::getline(os, desc);
			desc.erase(0, 1); // Erase first space.
			Variable var = statistics.getVariableByNameDescription(name, desc);
			const bool isNan = var.isNan();
			variables.push_back(std::move(var));

			if (isNan)
			{
				std::cout << "\tWARNING: variable not found: '" << name << "': '" << desc << "'" << std::endl;
			}
			else
				varCount += 1;
		}
		std::cout << "\tExpert " << getName() << " supplied with " << varCount << " variables." << std::endl;
	}

	void ExpertAdvisorExternal::execute(const std::time_t &secondsSinceStart, const std::time_t &time)
	{
		// Construct message made up from all the observed variables.
		Interfaces::ExpertMessage message;
		message.set_type(Interfaces::ExpertMessage_Type_getPrediction);
		for (const Variable &var : variables)
			message.add_variables(var.get());
		send(message);
		
		Interfaces::ExpertMessage reply = receive();
		
		setMood(reply.estimation().mood(), reply.estimation().certainty());
	}


	void ExpertAdvisorExternal::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		
	}

}; // namespace MM
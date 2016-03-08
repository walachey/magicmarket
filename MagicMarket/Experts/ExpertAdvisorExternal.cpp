#include "ExpertAdvisorExternal.h"
#include "Market.h"
#include "Statistics.h"
#include "Trade.h"
#include "Stock.h"

#include "Interfaces\Expert.pb.h"

#include <tuple>
#include <algorithm>

std::tuple<std::string, std::string> unwrapVariableNameDesc(std::string variableNameDesc)
{
	std::istringstream os(variableNameDesc);
	std::string name, desc;
	os >> name;
	std::getline(os, desc);
	desc.erase(0, 1);
	return std::make_tuple(name, desc);
}

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
		noPrediction = false;
	}

	void ExpertAdvisorExternal::declareExports() const
	{
		if (!noPrediction) ExpertAdvisor::declareExports();

		if (!providedVariableNames.empty())
		{
			size_t counter = 0;
			for (const std::string & variableNameDesc : providedVariableNames)
			{
				std::string name, desc;
				std::tie(name, desc) = unwrapVariableNameDesc(variableNameDesc);
				statistics.addVariable(Variable(name, &providedVariables[counter], desc));

				counter += 1;
			}
		}
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

		onAfterConnectionEstablished();

		std::cout << "\tExpert " << name << " linked." << std::endl;

		return true;
	}

	void ExpertAdvisorExternal::onAfterConnectionEstablished()
	{
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
			noPrediction = reply.information().noprediction();
			if (noPrediction)
			{
				std::cout << "\tAgent does not provide a prediction." << std::endl;
				if (executive)
				{
					executive = false;
					std::cout << "\t\tWARNING: Forcing role to 'supportive' (requested: 'executive')" << std::endl;
				}
			}
			
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

		{ // Provided variables
			Interfaces::ExpertMessage message;
			message.set_type(Interfaces::ExpertMessage_Type_getProvidedVariables);
			send(message);

			Interfaces::ExpertMessage reply = receive();
			assert(reply.type() == Interfaces::ExpertMessage_Type_getProvidedVariables);

			// IFF there are fresh observations, reserve some space for them and register them.
			providedVariableNames = std::vector<std::string>(reply.variablenames().begin(), reply.variablenames().end());
			providedVariables.resize(providedVariableNames.size());
			for (double & variable : providedVariables) variable = std::numeric_limits<double>::quiet_NaN();
			std::cout << "\tExpert is providing " << providedVariableNames.size() << " custom observations." << std::endl;
		}
	}

	void ExpertAdvisorExternal::afterExportsDeclared()
	{
		size_t varCount = 0;
		for (const std::string &namedesc : requiredVariables)
		{
			std::string name, desc;
			std::tie(name, desc) = unwrapVariableNameDesc(namedesc);
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
		// This can either result in a prediction or an update call, depending on what the agent wants.
		Interfaces::ExpertMessage::Type messageType = Interfaces::ExpertMessage::Type::ExpertMessage_Type_getPrediction;
		if (noPrediction)
			messageType = Interfaces::ExpertMessage::Type::ExpertMessage_Type_update;
		// Construct message made up from all the observed variables.
		Interfaces::ExpertMessage message;
		message.set_type(messageType);
		for (const Variable &var : variables)
			message.add_variables(var.get());
		send(message);
		
		Interfaces::ExpertMessage reply = receive();
		
		if (!noPrediction)
			setMood(reply.estimation().mood(), reply.estimation().certainty());
		else
		{
			assert(reply.variables_size() > 0);
			assert(reply.variables_size() == providedVariables.size());
			size_t i = 0, size = reply.variables_size();
			for (; i < size; ++i)
				providedVariables[i] = reply.variables(i);
		}
	}


	void ExpertAdvisorExternal::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		
	}

}; // namespace MM
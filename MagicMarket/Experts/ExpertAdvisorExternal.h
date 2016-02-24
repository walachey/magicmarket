#pragma once
#include "ExpertAdvisor.h"

#include <memory>
#include <zmq.hpp>


namespace Interfaces
{
	class ExpertMessage;
};

namespace MM
{
	struct Variable;

	class ExpertAdvisorExternal : public ExpertAdvisor
	{
	public:
		virtual void reset() override;
		ExpertAdvisorExternal();
		virtual ~ExpertAdvisorExternal();

		virtual std::string getName() const override { return name; };
		virtual void declareExports() const override;
		virtual bool isExecutive() const { return executive; }

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

		bool connect(std::string endpoint);
		virtual void afterExportsDeclared() override;
		virtual void onAfterConnectionEstablished();

	private:
		bool executive;
		bool noPrediction;
		std::string name;
		std::unique_ptr<zmq::context_t> context;
		std::unique_ptr<zmq::socket_t> socket;

		void send(Interfaces::ExpertMessage &message);
		Interfaces::ExpertMessage receive();

		std::vector<Variable> variables;
		std::vector<std::string> requiredVariables;
		std::vector<double> providedVariables;
	};

}
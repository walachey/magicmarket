#pragma once
#include "ExpertAdvisor.h"


namespace MM
{

	class ExpertAdvisorBroker : public ExpertAdvisor
	{
	public:
		ExpertAdvisorBroker();
		virtual ~ExpertAdvisorBroker();
		virtual void reset() override;

		virtual std::string getName() const override { return "lika"; };

		//virtual void execute(std::time_t secondsPassed);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);
		virtual std::vector<std::string> getRequiredExperts() const override { return{ "*" }; }
	private:
		std::time_t lastExecutionActionTime;
	};

};
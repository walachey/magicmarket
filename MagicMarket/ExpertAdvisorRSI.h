#pragma once
#include "ExpertAdvisor.h"

namespace MM
{
	class ExpertAdvisorRSI : public ExpertAdvisor
	{
	public:
		ExpertAdvisorRSI();
		virtual ~ExpertAdvisorRSI();

		virtual std::string getName() { return "karl"; };

		//virtual void execute(std::time_t secondsPassed);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

	};

};
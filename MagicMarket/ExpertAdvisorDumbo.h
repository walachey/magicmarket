#pragma once
#include "ExpertAdvisor.h"

namespace MM
{

	class ExpertAdvisorDumbo : public ExpertAdvisor
	{
	public:
	public:
		ExpertAdvisorDumbo();
		virtual ~ExpertAdvisorDumbo();

		virtual std::string getName() { return "dumbo"; };

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);
	};

}
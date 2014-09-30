#pragma once
#include "ExpertAdvisor.h"

#include "ExpertAdvisor.h"

namespace MM
{

	class ExpertAdvisorMAAnalyser : public ExpertAdvisor
	{
	public:
		ExpertAdvisorMAAnalyser();
		virtual ~ExpertAdvisorMAAnalyser();

		virtual std::string getName() { return "bob"; };

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

	private:
		std::time_t lastMASave;
	};

}
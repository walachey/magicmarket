#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{

	class ExpertAdvisorMAAnalyser : public ExpertAdvisorTechnical
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
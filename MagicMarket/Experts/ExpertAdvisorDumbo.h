#pragma once
#include "ExpertAdvisor.h"

namespace MM
{

	class ExpertAdvisorDumbo : public ExpertAdvisor
	{
	public:
	public:
		virtual void reset() override;
		ExpertAdvisorDumbo();
		virtual ~ExpertAdvisorDumbo();

		virtual std::string getName() const override { return "dumbo"; };

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);
	};

}
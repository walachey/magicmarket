#pragma once
#include "ExpertAdvisor.h"

namespace MM
{

	class ExpertAdvisorLimitAdjuster : public ExpertAdvisor
	{
	public:
		ExpertAdvisorLimitAdjuster();
		virtual ~ExpertAdvisorLimitAdjuster();

		virtual std::string getName() { return "ajeet"; };

		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);
		virtual bool acceptNewTrade(Trade *trade);
	};

}
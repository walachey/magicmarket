#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators 
	{
		class RSI;
	};

	class ExpertAdvisorRSI : public ExpertAdvisorTechnical
	{
	public:
		ExpertAdvisorRSI();
		virtual ~ExpertAdvisorRSI();

		virtual std::string getName() { return "karl"; };

		//virtual void execute(std::time_t secondsPassed);
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

	private:
		Indicators::RSI *rsiShort;
		Indicators::RSI *rsiLong;
	};

};
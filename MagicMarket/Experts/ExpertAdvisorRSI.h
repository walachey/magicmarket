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

		virtual std::string getName() { return "TA_RSI"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

	private:
		Indicators::RSI *rsiShort;
		Indicators::RSI *rsiLong;
	};

};
#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators 
	{
		class TSI;
	};

	class ExpertAdvisorTSI : public ExpertAdvisorTechnical
	{
	public:
		virtual void reset() override {}
		ExpertAdvisorTSI();
		virtual ~ExpertAdvisorTSI();

		virtual std::string getName() const override { return "TA_TSI"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

	private:
		Indicators::TSI *tsi;
	};

};
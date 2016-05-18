#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators 
	{
		class Renko;
	};

	class ExpertAdvisorRenko : public ExpertAdvisorTechnical
	{
	public:
		virtual void reset() override {}
		ExpertAdvisorRenko(double sensitivity = 5.0);
		virtual ~ExpertAdvisorRenko();

		virtual std::string getName() const override { return "TA_Renko"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

	private:
		Indicators::Renko *renko;
	};

};
#pragma once
#include "ExpertAdvisor.h"

namespace MM
{
	class ExpertAdvisorTechnical : public ExpertAdvisor
	{
	public:
		ExpertAdvisorTechnical();
		virtual ~ExpertAdvisorTechnical();

		virtual bool isTechnicalAgent() override { return true; }
	};

};
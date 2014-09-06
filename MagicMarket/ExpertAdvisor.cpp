#include "ExpertAdvisor.h"
#include "Market.h"

namespace MM
{
	ExpertAdvisor::ExpertAdvisor()
	{
	}


	ExpertAdvisor::~ExpertAdvisor()
	{
	}


	void ExpertAdvisor::say(std::string message)
	{
		market.chat(getName(), message);
	}
};
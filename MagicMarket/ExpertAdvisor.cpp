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
		if (message == lastMessage) return;
		lastMessage = message;

		market.chat(getName(), message);
	}
};
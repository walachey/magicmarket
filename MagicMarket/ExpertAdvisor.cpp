#include "ExpertAdvisor.h"
#include "Market.h"

namespace MM
{
	ExpertAdvisor::ExpertAdvisor()
	{
		lastMood = 0.0;
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

	void ExpertAdvisor::setMood(float mood, float certainty)
	{
		assert(certainty >= 0.0 && certainty <= 1.0);
		assert(mood >= -1.0 && mood <= 1.0);
		if ((mood == lastMood) && (certainty == lastCertainty)) return;
		
		lastMood = mood;
		lastCertainty = certainty;
		market.updateMood(getName(), mood, certainty);
	}
};
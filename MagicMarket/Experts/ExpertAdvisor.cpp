#include "ExpertAdvisor.h"
#include "Market.h"

#include "Statistics.h"

namespace MM
{
	void ExpertAdvisor::onNewDay()
	{
		lastMood = 0.0f;
		lastCertainty = 0.0f;

		reset();
	}

	ExpertAdvisor::ExpertAdvisor()
	{
	}

	ExpertAdvisor::~ExpertAdvisor()
	{
	}

	void ExpertAdvisor::say(std::string message)
	{
		if (message.empty()) return;
		if (message[0] != '@' && message == lastMessage) return;
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

	void ExpertAdvisor::declareExports() const
	{
		exportVariable(getName() + "_mood", std::bind(&ExpertAdvisor::getLastMood, this), "mood");
		exportVariable(getName() + "_cert", std::bind(&ExpertAdvisor::getLastCertainty, this), "certainty");
	}

	void ExpertAdvisor::exportVariable(std::string name, std::function<double()> accessor, std::string description) const
	{
		statistics.addVariable(Variable(name, accessor, description));
	}
};
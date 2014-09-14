#include "ExpertAdvisorAtama.h"

namespace MM
{
	ExpertAdvisorAtama::ExpertAdvisorAtama()
	{
	}


	ExpertAdvisorAtama::~ExpertAdvisorAtama()
	{
	}

	void ExpertAdvisorAtama::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		if (currencyPair != "EURUSD") return;

	}



	// ANN interface
	void ExpertAdvisorAtama::info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput)
	{

	}

	void ExpertAdvisorAtama::load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns)
	{

	}
};
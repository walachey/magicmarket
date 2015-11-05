#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators 
	{
		class StochasticOscillator;
	};

	class ExpertAdvisorStochasticOscillator : public ExpertAdvisorTechnical
	{
	public:
		virtual void reset() override {}
		ExpertAdvisorStochasticOscillator();
		virtual ~ExpertAdvisorStochasticOscillator();

		virtual std::string getName() const override { return "TA_StochOsc"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

	private:
		Indicators::StochasticOscillator *oscillator;
		double lastPercentK, lastPercentD;
	};

};
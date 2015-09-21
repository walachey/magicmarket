#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators
	{
		class SMA;
	};

	class ExpertAdvisorMAAnalyser : public ExpertAdvisorTechnical
	{
	public:
		virtual void reset() override;
		ExpertAdvisorMAAnalyser();
		virtual ~ExpertAdvisorMAAnalyser();

		virtual std::string getName() const override { return "bob"; };

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

	private:
		std::time_t lastMASave;
	};

}
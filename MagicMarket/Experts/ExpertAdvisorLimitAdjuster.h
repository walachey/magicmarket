#pragma once
#include "ExpertAdvisor.h"

namespace MM
{

	class ExpertAdvisorLimitAdjuster : public ExpertAdvisor
	{
	public:
		virtual void reset() override;
		ExpertAdvisorLimitAdjuster();
		virtual ~ExpertAdvisorLimitAdjuster();

		virtual std::string getName() const override { return "ajeet"; };

		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;
		virtual void declareExports() const override;
		virtual bool acceptNewTrade(Trade *trade);

	private:
		// provides information about hour of day
		int hourOfDay;
	};

}
#pragma once
#include "ExpertAdvisor.h"

#include <vector>

namespace MM
{
	struct Variable;

	class ExpertAdvisorMultiCurrency : public ExpertAdvisor
	{
	public:
		virtual void reset() override {}
		ExpertAdvisorMultiCurrency();
		virtual ~ExpertAdvisorMultiCurrency();

		virtual void afterExportsDeclared() override;

		virtual std::string getName() const override { return "MC_slave"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

		virtual std::vector<std::string> getRequiredExperts() const override { return {"MultiCurrencyANN"}; }
	private:
		std::vector<Variable> variables;
	};

};
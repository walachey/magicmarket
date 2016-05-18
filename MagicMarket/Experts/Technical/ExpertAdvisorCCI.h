#pragma once
#include "ExpertAdvisorTechnical.h"

namespace MM
{
	namespace Indicators 
	{
		class CCI;
		class SMA;
	};

	class ExpertAdvisorCCI : public ExpertAdvisorTechnical
	{
	public:
		virtual void reset() override;
		ExpertAdvisorCCI();
		virtual ~ExpertAdvisorCCI();

		virtual std::string getName() const override { return "TA_CCI"; };
		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override;

	private:
		Indicators::CCI *cciShort;
		Indicators::CCI *cciLong;

		Indicators::SMA* cciShortMA;
		Indicators::SMA* cciLongMA;
	};

};
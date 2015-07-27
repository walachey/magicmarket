#pragma once
#include "ExpertAdvisor.h"

namespace MM
{
	namespace Indicators
	{
		class Base : public ::MM::ExpertAdvisor
		{
		public:
			Base();
			virtual ~Base();

			virtual std::string getName() override { return "Indicator"; }
			virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override { update(secondsSinceStart, time); };
			virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time) override {};
			
			virtual Base *init();

			virtual bool operator== (const Base &other) const = 0;

		private:
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) = 0;
		};

	};
};
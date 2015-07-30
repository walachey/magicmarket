#pragma once
#include "ExpertAdvisor.h"
#include <vector>

namespace MM
{
	namespace Indicators
	{
		// wrapper to allow the template to be declared here without including Market.h
		class Base;
		std::vector<Base*> &getActiveIndicators();

		class Base : public ::MM::ExpertAdvisor
		{
		public:
			Base();
			virtual ~Base();

			virtual std::string getName() override { return "Indicator"; }
			virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) override { update(secondsSinceStart, time); };
			virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time) override {};
			virtual bool operator== (const Base &other) const = 0;

		private:
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) = 0;
		};

		// factory function to create new value-provider singletons
		// usage:: Indicators::get<Indicators::RSI>(...)
		template<class C, typename... Args> C* get(Args&&... args)
		{
			std::vector<Base*> &indicators = getActiveIndicators();
			C* candidate = new C(args...);
			// check if an indicator with our configuration already exists
			for (Base * &indicator : indicators)
			{
				if (*candidate == *indicator)
				{
					delete candidate;
					return static_cast<C*>(indicator);
				}
			}
			indicators.push_back(candidate);
			return candidate;
		};
	};
};
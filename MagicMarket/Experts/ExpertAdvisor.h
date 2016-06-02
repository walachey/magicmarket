#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <functional>

#include <ql/types.hpp>
#include <ql/time/date.hpp>

namespace MM
{
	class Trade;
	struct Variable;

	class ExpertAdvisor
	{
	public:
		ExpertAdvisor();
		virtual ~ExpertAdvisor();

		virtual std::string getName() const = 0;

		// Force derived classes to implement this.
		// It's called when a new day starts or on construction.
		virtual void reset() = 0;

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) {};
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time) {};
		virtual bool acceptNewTrade(Trade *trade) { return true; }

		virtual bool isTechnicalAgent() { return false; }
		// Only certain agents do not function as input features themselves but contribute directly to the outcome.
		virtual bool isExecutive() const { return false; }
		// Called by the market when a new day starts. Resets internal state.
		void onNewDay();

		void say(std::string);
		// >0 -> buy; <0 -> sell
		void setMood(float mood, float certainty);
		void setMood(double mood, double certainty)
		{
			return setMood(static_cast<float>(mood), static_cast<float>(certainty));
		};
		double getLastMood() const { return static_cast<double>(lastMood); };
		double getLastCertainty() const { return static_cast<double>(lastCertainty); };

		void exportVariable(std::string name, std::function<double ()> accessor, std::string description) const;
		virtual void declareExports() const;
		// Called after all exports have been declared.
		virtual void afterExportsDeclared() {};
		// Can be used to make sure that this expert is evaluated after the experts it depends on.
		virtual std::vector<std::string> getRequiredExperts() const { return{}; }
	private:
		std::string lastMessage;
		float lastMood, lastCertainty;
	};

};
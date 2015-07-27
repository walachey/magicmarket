#pragma once

#include <string>
#include <ctime>

#include <ql/types.hpp>
#include <ql/time/date.hpp>

namespace MM
{
	class Trade;

	class ExpertAdvisor
	{
	public:
		ExpertAdvisor();
		virtual ~ExpertAdvisor();

		virtual std::string getName() = 0;

		virtual void execute(const std::time_t &secondsSinceStart, const std::time_t &time) {};
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time) = 0;
		virtual bool acceptNewTrade(Trade *trade) { return true; }

		virtual bool isTechnicalAgent() { return false; }

		void say(std::string);
		// >0 -> buy; <0 -> sell
		void setMood(float mood, float certainty);
		float getLastMood() { return lastMood; };
		float getLastCertainty() { return lastCertainty; };
	private:
		std::string lastMessage;
		float lastMood, lastCertainty;
	};

};
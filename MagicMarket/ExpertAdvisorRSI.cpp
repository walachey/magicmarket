#include "ExpertAdvisorRSI.h"

#include <iomanip>

#include "Market.h"
#include "Stock.h"
#include "Tick.h"
#include "Helpers.h"

namespace MM
{
	ExpertAdvisorRSI::ExpertAdvisorRSI()
	{
	}


	ExpertAdvisorRSI::~ExpertAdvisorRSI()
	{
	}

	void ExpertAdvisorRSI::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		if (currencyPair != "EURUSD") return;

		Stock *stock = market.getStock(currencyPair);
		if (!stock) return;

		const int rsiPeriodLength = 28;
		const int rsiTimespans[] = { 1 * ONEMINUTE, 5 * ONEMINUTE };
		double rsiValues[] = { 0.0, 0.0 };
		const int timespanCount = 2;
		int validRSIEntries = 0;

		for (size_t timespanIndex = 0; timespanIndex < timespanCount; ++timespanIndex)
		{
			const int &timespan = rsiTimespans[timespanIndex];
			double winSum(0.0), lossSum(0.0);
			const int timeStart = time - rsiPeriodLength * timespan;
			std::vector<QuantLib::Decimal> wins, losses;
			wins.reserve(rsiPeriodLength);
			losses.reserve(rsiPeriodLength);

			TimePeriod period = stock->getTimePeriod(timeStart, time);
			std::vector<QuantLib::Decimal> closeValues = period.toVector(timespan);
			std::vector<QuantLib::Decimal> derivation = Math::derive(closeValues);

			for (auto &val : derivation)
			{
				if (val < 0.0)
					losses.push_back(val);
				else if (val > 0.0)
					wins.push_back(val);
			}

			winSum = Math::sum(wins);
			lossSum = Math::sum(losses);
			int winCount = wins.size();
			int lossCount = losses.size();
			int totalObersvations = derivation.size();
			if (totalObersvations == 0) continue;
			if (!winCount && !lossCount) continue;

			QuantLib::Decimal avgWin(winSum / (QuantLib::Decimal)totalObersvations);
			QuantLib::Decimal avgLoss(lossSum / (QuantLib::Decimal)totalObersvations);

			QuantLib::Decimal RS = avgWin / std::abs(avgLoss);

			QuantLib::Decimal RSI = QuantLib::Decimal(100) -
				(QuantLib::Decimal(100)
				/ (QuantLib::Decimal(1) + RS));
			
			if (avgWin == avgLoss) RSI = 50.0; // f.e. if both are 0
			else if (avgWin == 0.0) RSI = 0.0;
			else if (avgLoss == 0.0) RSI = 100.0;

			rsiValues[timespanIndex] = RSI;
			if (RSI != 0.0)
				++validRSIEntries;

			assert((rsiValues[timespanIndex] >= 0.0) && (rsiValues[timespanIndex] <= 100.0));
		}

		QuantLib::Decimal avgRSI = 0.0;
		for (const QuantLib::Decimal &rsi : rsiValues)
		{
			if (rsi == 0.0) continue;
			avgRSI += rsi;
		}
		avgRSI /= QuantLib::Decimal(timespanCount);

		QuantLib::Decimal margin = 30.0;

		market.updateParameter("RSI", avgRSI);

		if (validRSIEntries > 0)
		{
			if (avgRSI > (100.0 - margin))
				return setMood(-1.0, (avgRSI - (100.0 - margin)) / margin);
			if (avgRSI < (margin))
				return setMood(+1.0, (1.0 - (avgRSI / margin)));
		}
		setMood(0.0, 0.25);
		
	}

};
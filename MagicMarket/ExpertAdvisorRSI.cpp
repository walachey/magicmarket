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

		const int rsiPeriodLength = 14;
		const int rsiTimespans[] = { 5 * ONEMINUTE, 10 * ONEMINUTE };
		double rsiValues[] = { 0.0, 0.0 };
		const int timespanCount = 2;
		int validRSIEntries = 0;

		for (size_t timespanIndex = 0; timespanIndex < timespanCount; ++timespanIndex)
		{
			const int &timespan = rsiTimespans[timespanIndex];
			double winSum(0.0), lossSum(0.0);
			int winCount(0), lossCount(0);
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
			winCount = wins.size();
			lossCount = losses.size();

			QuantLib::Decimal avgWin(winSum);
			if (winCount != 0) winSum /= (QuantLib::Decimal)winCount;
			QuantLib::Decimal avgLoss(lossSum);
			if (lossCount != 0) lossSum /= (QuantLib::Decimal)lossCount;

			QuantLib::Decimal sum = avgWin + std::abs(avgLoss);
			QuantLib::Decimal RS = 0;
			if (sum != 0.0) RS = avgWin / sum;

			rsiValues[timespanIndex] = QuantLib::Decimal(100) - 
					(QuantLib::Decimal(100)
				/ (QuantLib::Decimal(1) + RS));
			if (rsiValues[timespanIndex] != 0.0)
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

		if (validRSIEntries > 0)
		{
			if (avgRSI > (100.0 - margin))
				return setMood(-1.0, (avgRSI - (100.0 - margin)) / margin);
			if (avgRSI < (margin))
				return setMood(+1.0, (1.0 - (avgRSI / margin)));
			std::ostringstream os; os << "RSI is " << std::setprecision(3) << avgRSI;
			say(os.str());
		}
		setMood(0.0, 0.0);
	}

};
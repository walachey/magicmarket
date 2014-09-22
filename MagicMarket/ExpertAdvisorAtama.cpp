#include "ExpertAdvisorAtama.h"

#include <ql/time/date.hpp>

#include <random>

#include "Stock.h"
#include "Trade.h"
#include "Market.h"



namespace MM
{
	ExpertAdvisorAtama::ExpertAdvisorAtama()
	{
		stocksToEvaluate = { "EURUSD", "USDDKK", "USDCHF", "EURCAD", "EURAUD", "EURJPY", "AUDCHF" };
		daysForInitialTraining = { 
			QuantLib::Date(11, (QuantLib::Month)(9 + 1), 2014),
			QuantLib::Date(12, (QuantLib::Month)(9 + 1), 2014),
			QuantLib::Date(15, (QuantLib::Month)(9 + 1), 2014),
			QuantLib::Date(16, (QuantLib::Month)(9 + 1), 2014),
			QuantLib::Date(17, (QuantLib::Month)(9 + 1), 2014),
			QuantLib::Date(18, (QuantLib::Month)(9 + 1), 2014)
			
		};

		// flush log file
		std::fstream log("saves/ANN.log", std::ios_base::out | std::ios_base::trunc);
	}


	ExpertAdvisorAtama::~ExpertAdvisorAtama()
	{
	}

	void ExpertAdvisorAtama::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		if (currencyPair != "EURUSD") return;

	}

	

	// ANN interface
	void ExpertAdvisorAtama::prepareTrainingData()
	{
		std::string errorMessage("");
		std::fstream log("saves/ANN.log", std::ios_base::out | std::ios_base::app);
		try
		{
			// get stock data
			std::vector<Stock*> stocks;
			for (std::string &stockName : stocksToEvaluate)
			{
				Stock *stock = market.getStock(stockName);
				if (stock == nullptr)
				{
					std::ostringstream os; os << "Could not load stock " << stockName;
					throw os.str();
				}
				stocks.push_back(stock);
			}

			// get all time periods that contain enough data to train the networks
			log << "Calculating time periods.." << std::endl;
			const int startingHour = 8, endingHour = 16;
			const int timeWindow = ONEHOUR;
			std::vector<TimePeriod> dataPeriods;

			for (QuantLib::Date &date : daysForInitialTraining)
			{
				std::time_t startTime = mktime(date, startingHour, 0, 0);
				std::time_t endTime = mktime(date, endingHour, 0, 0);
				std::time_t currentTime = startTime;
				TimePeriod currentTimePeriod = TimePeriod(nullptr, currentTime, currentTime + timeWindow, nullptr);

				bool success = true;
				for (Stock *&stock : stocks)
				{
					currentTimePeriod.setStock(stock);
					int gapSeconds = currentTimePeriod.getMaximumSecondsBetweenTicks();

					if (gapSeconds > ONEMINUTE * 10)
					{
						// reset time period to last sane value and save if applicable
						currentTimePeriod.expandEndTime(-timeWindow);
						if (currentTimePeriod.getEndTime() - currentTimePeriod.getStartTime() >= timeWindow/2)
						{
							log << "Atama Warning: " << stock->getCurrencyPair() << " has ins. data @ " << date << "/" << currentTime << std::endl;
							dataPeriods.push_back(currentTimePeriod);
							success = false;
							break;
						}
					}
				}

				if (success) // try to expand the time period
				{
					currentTime += timeWindow;
					currentTimePeriod.expandEndTime(+timeWindow);
				}
				else // start a new time period
				{
					currentTime = currentTimePeriod.getEndTime() + timeWindow;
					currentTimePeriod = TimePeriod(nullptr, currentTime, currentTime + timeWindow, nullptr);
				}
			}

			// we now have all time periods with sufficient data
			log << "Time Periods:\t" << dataPeriods.size() << std::endl;
			int totalSeconds = 0;
			for (TimePeriod &period : dataPeriods)
			{
				totalSeconds += period.getEndTime() - period.getStartTime();
			}
			log << "Data Minutes:\t" << totalSeconds / 60 << std::endl;


			const int sampleCount = 2000;
			std::random_device rd;
			std::default_random_engine randomEngine(rd());
			std::uniform_int_distribution<int> randomSecond(0, totalSeconds);

			for (int sampleNumber = 0; sampleNumber < sampleCount; ++sampleNumber)
			{
				int second = randomSecond(randomEngine);

				// figure out time period that contains the second
				int currentSecond = 0;
				TimePeriod *period = nullptr;

				for (TimePeriod &toCheck : dataPeriods)
				{
					int duration = toCheck.getDuration();
					currentSecond += duration;
					if (currentSecond < second) continue;
					period = &toCheck;
					break;
				}
				assert(period);

				// we now have a random period to take a sample from
				std::time_t duration = period->getDuration();
				assert(duration > 30 * ONEMINUTE);
				std::time_t randomSampleTime = std::uniform_int_distribution<int>(30 * ONEMINUTE, duration)(randomEngine);

				// now get the input vector for that time & day
			}

		}
		catch (const char * s)
		{
			errorMessage = s;
		}
		catch (std::string s)
		{
			errorMessage = s;
		}

		if (!errorMessage.empty())
		{
			log << "FATAL: " << errorMessage << std::endl;
			std::cerr << "Atama: ERROR \"" << errorMessage << "\"" << std::endl;
			say(errorMessage);
		}
	}

	std::vector<float> ExpertAdvisorAtama::getInputVector(std::vector<Stock*> &stocks, const std::time_t &time)
	{
		std::vector<float> results;
		results.reserve(stocks.size() * 7 * 2);
		for (Stock *&stock : stocks)
		{
			std::vector<float> stockResults;
			stockResults = getInputVectorForStock(stock, time);

			for (float &f : stockResults)
				results.push_back(f);
		}
		return results;
	}

	std::vector<float> ExpertAdvisorAtama::getInputVectorForStock(Stock *stock, const std::time_t &time)
	{
		std::vector<int> timePeriods = { 10 * ONESECOND, 30 * ONESECOND, ONEMINUTE, 5 * ONEMINUTE, 10 * ONEMINUTE, 30 * ONEMINUTE, ONEHOUR };
		std::vector<float> results;
		results.resize(timePeriods.size() * 2); // Low / High

		for (size_t i = 0, ii = results.size(); i < ii; ++i)
			results[i] = 0.0f;

		size_t indexCounter = 0;
		for (const int &periodDuration : timePeriods)
		{
			TimePeriod period = stock->getTimePeriod(time - periodDuration, time);
			PossibleDecimal close = period.getClose();
			PossibleDecimal open = period.getOpen();
			assert(close && open);
			float delta =  *close - *open;
			if (delta > 0.0f) results[indexCounter] = delta;
			else results[indexCounter] = -delta;
			indexCounter += 2;
		}
		return results;
	}

	void ExpertAdvisorAtama::info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput)
	{

	}

	void ExpertAdvisorAtama::load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns)
	{

	}
};
#include "ExpertAdvisorAtama.h"

#include <ql/time/date.hpp>

#include <random>
#include <algorithm>

#include <trainer.h>
#include "Stock.h"
#include "Trade.h"
#include "Market.h"

#include <filesystem>
namespace filesystem = std::tr2::sys;


namespace MM
{
	const int ExpertAdvisorAtama::inputValuesPerStock = 7 * 2;

	ExpertAdvisorAtama::TrainingData::TrainingData(int noInput, int noOutput)
	{
		inputValues = (float*) malloc(noInput * sizeof(float));
		outputValues = (float*) malloc(noOutput * sizeof(float));
	}

	ExpertAdvisorAtama::TrainingData::~TrainingData()
	{
		free(inputValues);
		free(outputValues);
	}

	ExpertAdvisorAtama::ExpertAdvisorAtama()
	{
		currentState = State::NONE;
		ANN = nullptr;

		stocksToEvaluate = { "EURUSD", "USDDKK", "USDCHF", "EURCAD", "EURAUD", "EURJPY", "AUDCHF" };
		daysForInitialTraining = { 
			QuantLib::Date(11, (QuantLib::Month)(9), 2014),
			QuantLib::Date(12, (QuantLib::Month)(9), 2014),
			QuantLib::Date(15, (QuantLib::Month)(9), 2014),
			QuantLib::Date(16, (QuantLib::Month)(9), 2014),
			QuantLib::Date(17, (QuantLib::Month)(9), 2014),
			QuantLib::Date(18, (QuantLib::Month)(9), 2014),
			QuantLib::Date(22, (QuantLib::Month)(9), 2014),
			QuantLib::Date(23, (QuantLib::Month)(9), 2014)
			
		};

		// flush log file
		std::fstream log("saves/ANN.log", std::ios_base::out | std::ios_base::trunc);
		

		// get stock data
		for (std::string &stockName : stocksToEvaluate)
		{
			Stock *stock = market.getStock(stockName);
			if (stock == nullptr)
			{
				std::ostringstream os; os << "Could not load stock " << stockName;
				log << os.str();
				return;
			}
			stocks.push_back(stock);
		}
		log.close();

		currentState = State::LOADING;
		
		std::string saveFilename("saves/ANN.bin");
		filesystem::path savePath(saveFilename);
		bool training = true;

		if (filesystem::exists(savePath))
		{
			ANN = new network(saveFilename.c_str(), false);
			
			// training = false;			
		}
		
		if (training)
		{
			prepareTrainingData();
			currentState = State::TRAINING;
			executeTraining();

			ANN->textsave(saveFilename.c_str());
		}
		currentState = State::READY;
	}


	ExpertAdvisorAtama::~ExpertAdvisorAtama()
	{
		for (auto &data : trainingData)
		{
			delete data;
		}
	}

	void ExpertAdvisorAtama::onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time)
	{
		if (currencyPair != "EURUSD") return;
		if (currentState != State::READY) return;

		TrainingData data(stocks.size() * inputValuesPerStock, 3);
		getInputVector(stocks, time, &data);
		ANN->compute(data.inputValues, data.outputValues);

		float sum = 0.0f;
		for (int i = 0; i < 3; ++i)
			sum += data.outputValues[i];
		float &buy = data.outputValues[0];
		float &sell = data.outputValues[1];
		float &idle = data.outputValues[2];

		float mood = 0.0f;
		float value = 0.0f;
		if (std::max(sell, idle) < buy)
		{
			mood = +1.0f;
			value = buy;
		}
		else if (std::max(buy, idle) < sell)
		{
			mood = -1.0f;
			value = sell;
		}
		market.updateParameter("ANN", sum);
		if (sum != 0.0f) value /= sum;
		setMood(mood, value);
	}

	

	// ANN interface

	void ExpertAdvisorAtama::executeTraining()
	{
		std::fstream log("saves/ANN.log", std::ios_base::out | std::ios_base::app);

		if (!ANN)
		{
			ANN = new network(network::LOGISTIC, 4,
				stocksToEvaluate.size() * inputValuesPerStock,
				stocksToEvaluate.size() * (inputValuesPerStock - 2),
				stocksToEvaluate.size() * (inputValuesPerStock - 4),
				3);
			ANN->randomize(0.5);
		}

		trainer ANNTrainer(ANN, "saves/ANN_error.log", "saves/ANN_accuracy.log");
		ANNTrainer.set_iomanager(this);
		ANNTrainer.set_max_epochs(2500);
		ANNTrainer.set_min_error(0.1);

		ANNTrainer.load_training("RAM");
		int epochs = ANNTrainer.train_online(true);

		log << "TRAINING EPOCH\t" << ANNTrainer.get_current_epoch() << std::endl;
		ANNTrainer.test();
		log << "TRAINING ERROR\t" << ANNTrainer.get_error_on_training() << std::endl;
		log << "TRAINING ACCURACY\t" << ANNTrainer.get_accuracy_on_training() << std::endl;
	}

	void ExpertAdvisorAtama::prepareTrainingData()
	{
		assert(stocks.size());

		std::string errorMessage("");
		std::fstream log("saves/ANN.log", std::ios_base::out | std::ios_base::app);
		try
		{
			// get all time periods that contain enough data to train the networks
			log << "Calculating time periods.." << std::endl;
			const int startingHour = 8+2, endingHour = 16+2;
			const int timeWindow = ONEHOUR;
			std::vector<TimePeriod> dataPeriods;

			for (QuantLib::Date &date : daysForInitialTraining)
			{
				std::time_t startTime = mktime(date, startingHour, 0, 0);
				
				std::time_t endTime = mktime(date, endingHour, 0, 0);
				std::time_t currentTime = startTime;
				TimePeriod currentTimePeriod = TimePeriod(nullptr, currentTime, currentTime + timeWindow, nullptr);

				while (currentTime < endTime)
				{
					std::tm *tm = std::gmtime(&currentTime);
					printf_s("\rTIME\t%02d:%02d\t\t%02d.%02d.%04d", tm->tm_hour, tm->tm_min, tm->tm_mday, tm->tm_mon + 1, 1900 + tm->tm_year);
					bool success = true;
					for (Stock *&stock : stocks)
					{
						currentTimePeriod.setStock(stock);
						int gapSeconds = currentTimePeriod.getMaximumSecondsBetweenTicks();

						if ((gapSeconds > ONEMINUTE * 10) || (gapSeconds == -1))
						{
							if (currentTimePeriod.getEndTime() - currentTimePeriod.getStartTime() >= timeWindow / 2)
								log << "Atama Warning: " << stock->getCurrencyPair() << " has ins. data @ " << date << "/" << currentTime << std::endl;
							success = false;
							break;
						}
					}

					bool trySave = !success || (currentTime + timeWindow >= endTime);

					if (trySave)
					{
						// reset time period to last sane value and save if applicable
						currentTimePeriod.expandEndTime(-timeWindow);
						if (currentTimePeriod.getEndTime() - currentTimePeriod.getStartTime() >= timeWindow / 2)
						{
							dataPeriods.push_back(currentTimePeriod);
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
			}

			// we now have all time periods with sufficient data
			log << "Time Periods:\t" << dataPeriods.size() << std::endl;
			int totalSeconds = 0;
			for (TimePeriod &period : dataPeriods)
			{
				totalSeconds += period.getEndTime() - period.getStartTime();
			}
			log << "Data Minutes:\t" << totalSeconds / 60 << std::endl;


			const int sampleCount = 50000;
			std::random_device rd;
			std::default_random_engine randomEngine(rd());
			std::uniform_int_distribution<int> randomSecond(0, totalSeconds);

			for (int sampleNumber = 0; sampleNumber < sampleCount; ++sampleNumber)
			{
				int second = randomSecond(randomEngine);

				printf_s("\rSAMPLING %d%% %5d/%d @ %d", 100 * sampleNumber / sampleCount, sampleNumber, sampleCount, second);

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
				std::time_t evaluationTime = period->getStartTime() + randomSampleTime;

				// now get the input vector for that time & day
				TrainingData *data = new TrainingData(stocks.size() * inputValuesPerStock, 3);
				bool result = getInputVector(stocks, evaluationTime, data);

				// and check what the resulting action should be
				if (result == true)
					result = getOutputVector(stocks.front(), evaluationTime, data);

				// data set OK
				if (result == true)
					trainingData.push_back(data);
			}
			
			log << "Training Sets:\t" << trainingData.size() << std::endl;
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

	bool ExpertAdvisorAtama::getOutputVector(Stock *stock, const std::time_t &time, TrainingData *trainingData)
	{
		assert(stock->getCurrencyPair() == "EURUSD");
		
		for (size_t i = 0; i < 3; ++i)
			trainingData->outputValues[i] = 0.0f;

		TimePeriod period = stock->getTimePeriod(time, time + 10 * ONEMINUTE);
		PossibleDecimal open, high, low;
		open = period.getOpen();
		high = period.getHigh();
		low = period.getLow();

		if (!open || !high || !low) return false;

		QuantLib::Decimal highDiff = *high - *open;
		QuantLib::Decimal lowDiff = *open - *low;

		QuantLib::Decimal margin = 6.0 * ONEPIP;

		bool highOK = highDiff > margin;
		bool lowOK = lowDiff > margin;

		// invalid?
		if (highOK == lowOK)
		{
			trainingData->outputValues[2] = 1.0f;
		}
		// dis better be good
		else if (highOK)
		{
			trainingData->outputValues[0] = 1.0f;
		}
		else if (lowOK)
		{
			trainingData->outputValues[1] = 1.0f;
		}
		return true;
	}

	bool ExpertAdvisorAtama::getInputVector(std::vector<Stock*> &stocks, const std::time_t &time, TrainingData *trainingData)
	{
		float *current = trainingData->inputValues;
		for (Stock *&stock : stocks)
		{
			bool valid = getInputVectorForStock(stock, time, current);
			if (!valid) return false;
			
			current += inputValuesPerStock;
		}
		return true;
	}

	bool ExpertAdvisorAtama::getInputVectorForStock(Stock *stock, const std::time_t &time, float *inputData)
	{
		std::vector<int> timePeriods = { 10 * ONESECOND, 30 * ONESECOND, ONEMINUTE, 5 * ONEMINUTE, 10 * ONEMINUTE, 30 * ONEMINUTE, ONEHOUR };

		size_t indexCounter = 0;
		for (const int &periodDuration : timePeriods)
		{
			inputData[indexCounter] = 0.0f;
			inputData[indexCounter+1] = 0.0f;

			TimePeriod period = stock->getTimePeriod(time - periodDuration, time);
			PossibleDecimal close = period.getClose();
			PossibleDecimal open = period.getOpen();
			if (!(close && open)) return false;

			float delta =  *close - *open;
			if (delta > 0.0f) inputData[indexCounter] = delta;
			else inputData[indexCounter+1] = -delta;
			indexCounter += 2;
		}
		return true;
	}

	void ExpertAdvisorAtama::info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput)
	{
		assert(filename == "RAM");

		*npatterns = trainingData.size();
		*ninput = inputValuesPerStock * stocksToEvaluate.size();
		*noutput = 3;
	}

	void ExpertAdvisorAtama::load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns)
	{
		assert(npatterns == trainingData.size());
		assert(ninput == inputValuesPerStock * stocksToEvaluate.size());
		assert(noutput == 3);

		assert(npatterns <= trainingData.size());

		for (int i = 0; i < npatterns; ++i)
		{
			for (int in = 0; in < ninput; ++in)
				inputs[i][in] = trainingData[i]->inputValues[in];
			for (int out = 0; out < noutput; ++out)
				targets[i][out] = trainingData[i]->outputValues[out];
		}
	}
};
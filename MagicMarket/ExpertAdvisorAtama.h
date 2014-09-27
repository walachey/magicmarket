#pragma once
#include "ExpertAdvisor.h"

// for some fucking stupid reason the following two files use /using namespace std;/
//namespace ANN {
#include <network.h>
#include <iomanage.h>
//};

/*
	Setup of the neural networks:
	Input:
		(10sLH 30sLH 1mLH 5mLH 10mLH 30mLH 1hLH) * |currency pairs|
		LH are two neurons, one for negative values and one for positive.
		All values represent the first derivation of the stock (~change).
		All values are normalized to the highest LH value.
	Hidden Layers:
		Two hidden layers. One |currency pairs| * 2 neurons smaller. Another one |currency pairs| * 4 neurons smaller.
		Prior to the "normal" training, the hidden layers have been trained to resemble the original output.
	Output:
		Buy Sell NoAction

	Training:
		First setup: train hidden layers to resemble original input; then train net normally.
		Training needs to be repeated regularly during the day; excluding the deep learning stage.
*/

namespace MM
{
	class Stock;

	class ExpertAdvisorAtama : public ExpertAdvisor, private iomanage
	{
		class TrainingData
		{
		public:
			float *inputValues;
			float *outputValues;

			TrainingData(int noInput, int noOutput);
			TrainingData(const TrainingData &other); // undefined
			~TrainingData();
		};
	public:
		ExpertAdvisorAtama();
		virtual ~ExpertAdvisorAtama();

		virtual std::string getName() { return "atama"; };

		//virtual void execute(std::time_t secondsPassed);
		virtual void onNewTick(const std::string &currencyPair, const QuantLib::Date &date, const std::time_t &time);

		// implement methods to server as a trainer for the neural networks
		virtual void info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput);
		virtual void load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns);

		static const int inputValuesPerStock;

	private:
		enum State
		{
			NONE,
			LOADING,
			DEEP_PHASE1,
			DEEP_PHASE2,
			TRAINING,
			READY
		} currentState;
		network *ANN;
		std::vector<TrainingData*> trainingData;
		std::vector<std::string> stocksToEvaluate;
		std::vector<Stock*> stocks;
		std::vector<QuantLib::Date> daysForInitialTraining;
		void prepareTrainingData();
		void executeTraining();
		bool getInputVector(std::vector<Stock*> &stocks, const std::time_t &time, TrainingData *trainingData);
		bool getInputVectorForStock(Stock *stock, const std::time_t &time, float *inputData);
		bool getOutputVector(Stock *stock, const std::time_t &time, TrainingData *trainingData);
	};

};
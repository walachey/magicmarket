#include "DeepLearningTest.h"
#include "DeepLearningNetwork.h"
#include <fstream>
#include <iostream>
/*
	HACK for csv.h to work with VS without snprintf...

#include <cstdio>
#ifdef _MSC_VER
#include <stdarg.h>
#define snprintf c99_snprintf
namespace std
{
	inline int c99_snprintf(char* str, size_t size, const char* format, ...)
	{
		int count;
		va_list ap;

		va_start(ap, format);
		count = c99_vsnprintf(str, size, format, ap);
		va_end(ap);

		return count;
	}

	inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
	{
		int count = -1;

		if (size != 0)
			count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
		if (count == -1)
			count = _vscprintf(format, ap);

		return count;
	}
};

#endif // _MSC_VER
#include <csv\csv.h>
*/
#include <QuantLib\ql\time\all.hpp>

#include "Market.h"
#include "TimePeriod.h"
#include "Helpers.h"

namespace MM
{
	DeepLearningTest::DeepLearningTest()
	{
	}


	DeepLearningTest::~DeepLearningTest()
	{
	}


	void DeepLearningTest::run()
	{
		std::vector<QuantLib::Date> daysForInitialTraining = {
			QuantLib::Date(11, (QuantLib::Month)(9), 2014),
			QuantLib::Date(12, (QuantLib::Month)(9), 2014),
			QuantLib::Date(15, (QuantLib::Month)(9), 2014),
			QuantLib::Date(16, (QuantLib::Month)(9), 2014),
			QuantLib::Date(17, (QuantLib::Month)(9), 2014),
			QuantLib::Date(18, (QuantLib::Month)(9), 2014),
			QuantLib::Date(22, (QuantLib::Month)(9), 2014),
			QuantLib::Date(23, (QuantLib::Month)(9), 2014)

		};
		const int startingHour = 8 + 2, endingHour = 16 + 2;
		const int timeWindow = 1 * ONEMINUTE;
		const int windowCount = ONEMINUTE * 30 / timeWindow;
		Stock *stock = market.getStock("EURUSD");

		std::fstream sampleOutput("saves/deeplearning/samples.tsv", std::ios_base::out);
		int goodTrainingSamples(0), badTrainingSamples(0);
		int totalTicks, totalChanges;

		// Load data
		std::vector<std::vector<float>> trainingData;
		std::cout << "PREPARING DEEP NETWORK-------------------------" << std::endl;
		for (QuantLib::Date &date : daysForInitialTraining)
		{
			std::time_t startTime = mktime(date, startingHour, 0, 0);

			std::time_t endTime = mktime(date, endingHour, 0, 0);
			std::time_t currentTime = startTime;

			for (; currentTime + timeWindow * windowCount < endTime; currentTime += timeWindow)
			{
				// Check validity of period
				TimePeriod currentTimePeriod = TimePeriod(stock, currentTime, currentTime + timeWindow * windowCount, nullptr);
				const int maxSeconds = currentTimePeriod.getMaximumSecondsBetweenTicks(&totalTicks, &totalChanges);
				sampleOutput << maxSeconds << "\t" << totalTicks << "\t" << totalChanges << std::endl;

				if (maxSeconds > 15 * ONESECOND || totalTicks < 5 || totalChanges < 5)
				{
					badTrainingSamples += 1;
					continue;
				}

				goodTrainingSamples += 1;

				if ((goodTrainingSamples + badTrainingSamples) % 100 == 0)
				{
					std::cout << "GOOD SAMPLES:\t" << goodTrainingSamples << "\t\t" << "BAD SAMPLES:\t" << badTrainingSamples << "\t\r";
				}

				std::vector<double> data = currentTimePeriod.toVector(timeWindow);
				const int maxSeconds2 = currentTimePeriod.getMaximumSecondsBetweenTicks(&totalTicks, &totalChanges);
				assert(data.size() == windowCount + 1);
				auto dataDerivate = Math::derive(data);
				Math::normalize(dataDerivate);
				trainingData.push_back(toFloatVector(dataDerivate));
				assert(Math::stddev(trainingData.back()) > 0.0);
			}
		}
		sampleOutput.close();
		std::cout << std::endl << "DONE LOADING DATA---------------------------" << std::endl;
		std::cout << "TRAINING DATA-----------------------------" << std::endl;
		std::cout << "Samples\t\t\t" << goodTrainingSamples << std::endl;
		std::cout << "Bad Samples\t\t" << badTrainingSamples << std::endl;
		std::cout << "------------------------------------------" << std::endl;

		::AI::DeepLearningNetwork network;
		network.setLayerSetup({ windowCount , 20, 16, 8, 4});
		network.setTrainingInput(trainingData);
		network.train();

		std::cout << "\n\nDONE--------------------------------" << std::endl;
		getchar();
	}
};
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <functional>
#include <algorithm>

std::vector<std::string> split(const std::string& input, const std::regex& regex)
{
	// passing -1 as the submatch index parameter performs splitting
	std::sregex_token_iterator first { input.begin(), input.end(), regex, -1 }, last;
	return{ first, last };
}

int main(int argc, char ** argv)
{
	std::vector<std::string> arguments(argv + 1, argv + argc);

	struct _config
	{
		std::string outputFilename;
		std::string filename;
		std::regex  delimiter;
		int         skipRows;
		int         timedeltaSeconds;

		_config() : delimiter(","), skipRows(0), timedeltaSeconds(-1)
		{
			
		};
	} config;

	// Parse command-line args.
	std::map<std::string, std::function<void(std::string)>> argumentMapping =
	{
		{ "--filename", [&](std::string s)
						{
							config.filename = s;
						}
		},
		{ "--delimiter", [&](std::string s)
						{
							config.delimiter = std::regex(s);
						}
		},
		{ "--skiprows", [&](std::string s)
						{
							config.skipRows = std::stoi(s);
						}
		},
		{ "--outfile", [&](std::string s)
						{
							config.outputFilename = s;
						}
		},
		{ "--timedelta", [&](std::string s)
						{
							config.timedeltaSeconds = std::stoi(s);
						}
		},
	};

	for (auto & option : argumentMapping)
	{
		std::vector<std::string>::iterator iter = std::find(arguments.begin(), arguments.end(), option.first);
		if (iter == arguments.end()) continue;
		if (++iter == arguments.end() || iter->size() == 0 || (*iter)[0] == '-')
		{
			std::cerr << "ERROR: missing argument value for " << option.first << std::endl;
			continue;
		}
		option.second(*iter);
	}

	if (config.timedeltaSeconds <= 0)
	{
		std::cerr << "ERROR: timedelta not specified or invalid (--timedelta)." << std::endl;
		exit(1);
	}

	std::istream *input = nullptr;

	if (config.filename.size() == 0)
	{
		std::cerr << "INFO: no filename supplied using std::in" << std::endl;
		input = &std::cin;
	}
	else
	{
		input = new std::fstream(config.filename, std::ios_base::in);
		if (!input->good())
		{
			std::cerr << "ERROR: could not open input file" << std::endl;
			return 1;
		}
	}

	std::ostream *output;
	if (config.outputFilename.empty())
	{
		output = &std::cout;
	}
	else
	{
		if (config.outputFilename == "auto")
		{
			config.outputFilename = config.filename + ".period" + std::to_string(config.timedeltaSeconds) + "s.csv";
		}

		output = new std::ofstream(config.outputFilename, std::ios_base::out | std::ios_base::trunc);
	}

	(*output) << "index,timestamp,open,close,high,low,spread,volatility" << std::endl;

	// cache
	std::vector<std::vector<std::string>> currentTicks;

	struct _lastPeriod
	{
		double close;
		double spread;
		std::string originalTickIndex;

		_lastPeriod() : close(0.0), spread(0.0) {};
	} lastPeriod;

	auto pushPeriod = [&](decltype(currentTicks) ticks, long timestamp)
	{
		if (ticks.empty())
		{
			std::cerr << "ERROR: period without ticks during day - timestamp " << timestamp << std::endl;
			exit(1);
		}

		std::vector<double> mids;
		mids.reserve(ticks.size());

		double spreadSum = 0.0;

		for (auto &tick : ticks)
		{
			const double bid = std::stod(tick[2]);
			const double ask = std::stod(tick[3]);

			const double mid = (bid + ask) / 2.0;
			const double spread = (ask - bid);

			mids.push_back(mid);
			spreadSum += spread;
		}

		spreadSum /= static_cast<double>(ticks.size());
		const std::string &originalTickIndex = ticks.back()[0];
		const double &open = mids.front();
		const double &close = mids.back();
		const long volatility = ticks.size();
		
		const double high = *std::max_element(mids.begin(), mids.end());
		const double low  = *std::min_element(mids.begin(), mids.end());

		(*output) << originalTickIndex << ","
			<< timestamp << ","
			<< open << ","
			<< close << ","
			<< high << ","
			<< low << ","
			<< spreadSum << ","
			<< volatility << std::endl;

		lastPeriod.originalTickIndex = originalTickIndex;
		lastPeriod.close = close;
		lastPeriod.spread = spreadSum;
	};

	auto replicatePeriod = [&](long timestamp)
	{
		(*output) << lastPeriod.originalTickIndex << ","
			<< timestamp << ","
			<< lastPeriod.close << ","
			<< lastPeriod.close << ","
			<< lastPeriod.close << ","
			<< lastPeriod.close << ","
			<< lastPeriod.spread << ","
			<< 0 << std::endl;
	};

	const int timedeltaSeconds = config.timedeltaSeconds;

	const int startingHour = 8;
	const int closingHour  = 18;

	int periodBeginTickIndex = 0;
	long currentTimeSeconds  = 0;

	long lastTickDatetimeSeconds = 0;

	bool newDay = true;

	std::string line;
	int lineCounter = 0;
	while (std::getline(*input, line))
	{
		if (++lineCounter <= config.skipRows) continue;
		if (line.size() == 0) continue;

		std::vector <std::string> fields = split(line, config.delimiter);

		const std::string &origIndex     = fields[0];
		const std::string &origTimestamp = fields[1];
		const std::string &origHour      = fields[7];

		const long tickIndex = std::stoi(origIndex);
		const long timestampSeconds = std::stol(origTimestamp);
		const int hour = std::stoi(origHour);

		auto reset = [&]()
		{
			periodBeginTickIndex = tickIndex;
			currentTicks.clear();
		};

		auto doStep = [&]()
		{
			currentTimeSeconds += timedeltaSeconds;
			reset();
		};

		// Fast forward to begin of day.
		if (hour < startingHour || hour > closingHour)
		{
			newDay = true;
			continue;
		}

		// Begin a fresh day?
		if (newDay)
		{
			reset();
			currentTimeSeconds = timestampSeconds + timedeltaSeconds;
			newDay = false;
			currentTicks.push_back(fields);
			continue;
		}

		// If target time passed, remember period information.
		if (timestampSeconds > currentTimeSeconds)
		{
			// need to catch up? did we miss periods?
			while (timestampSeconds > currentTimeSeconds + timedeltaSeconds)
			{
				replicatePeriod(currentTimeSeconds);
				currentTimeSeconds += timedeltaSeconds;
			}
			pushPeriod(currentTicks, currentTimeSeconds);
			doStep();
		}

		currentTicks.push_back(fields);
	}


	if (input != &std::cin)
		delete input;

	if (output != &std::cout)
		delete output;

	return 0;
}
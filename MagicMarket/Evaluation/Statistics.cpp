#include "Statistics.h"

#include "Market.h"
#include <SimpleIni.h>
#include "VirtualMarket.h"

#include <sstream>

MM::Statistics statistics;

namespace MM
{
	std::string Variable::getS() const
	{
		const double val = get();
		if (std::isnan(val)) return "nan";
		else return std::to_string(val);
	}

	Variable Variable::NaN()
	{
		return std::move(Variable("NaN", std::function<double()>([]() { return std::numeric_limits<double>::quiet_NaN(); }), "Always returns NaN."));
	}

	Statistics::Statistics()
	{
	}


	Statistics::~Statistics()
	{
	}

	void Statistics::init(void *_ini)
	{
		const CSimpleIniA &ini = *(CSimpleIniA*)_ini;
		loggingActive = ini.GetBoolValue("Statistics", "Enabled", true);
		config.outputFilename = ini.GetValue("Statistics", "OutputFilename", "saves/variables.csv");
		config.delimiter = ini.GetValue("Statistics", "Delimiter", "\t");
		if (config.delimiter == "tab")
			config.delimiter = "\t";
		else if (config.delimiter == "space")
			config.delimiter = " ";
	}

	void Statistics::addVariable(const Variable &var)
	{
		const size_t duplicateCount = std::count_if(variables.begin(), variables.end(), [&](const Variable &other) { return other.originalName == var.name; }); 

		variables.push_back(var);

		if (duplicateCount > 0)
		{
			std::ostringstream os;
			os << var.name << "_" << (duplicateCount + 1);
			variables.back().name = os.str();
		}
	}

	Variable Statistics::getVariableByNameDescription(std::string name, std::string desc) const
	{
		auto found = std::find_if(variables.begin(), variables.end(), [&](const Variable &var) { return var.originalName == name && var.description == desc; });
		if (found != variables.end()) return *found;

		// Return a NaN-variable
		return Variable::NaN();
	}

	void Statistics::log()
	{
		if (!loggingActive) return;

		// try to open for the very first time?
		if (!outputStream.is_open())
		{
			// log variable descriptions
			std::fstream descriptionStream(config.outputFilename + ".info", std::ios_base::out);
			// open real stream
			outputStream.open(config.outputFilename, std::ios_base::out);
			if (!outputStream.good()) return;

			// print header row
			descriptionStream << "name" << config.delimiter << "description" << std::endl;
			outputStream << "time";
			for (const Variable &var : variables)
			{
				outputStream << config.delimiter << var.name;

				descriptionStream << var.name << config.delimiter << var.originalName << " " << var.description << std::endl;
			}
			outputStream << std::endl << std::flush;
		}

		// get all observations and log them
		outputStream << market.getLastTickTime();

		for (const Variable &var : variables)
		{
			outputStream << config.delimiter << var.getS();
		}
		// flush file to allow killing the application without losses
		outputStream << std::endl << std::flush;
	}
};
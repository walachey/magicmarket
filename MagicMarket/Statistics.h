#pragma once

#include <string>
#include <map>
#include <vector>

#include <functional>
#include <fstream>

namespace MM
{

	struct Variable
	{
		Variable(std::string name, std::function<double()> accessor, std::string description) : 
			name(name), originalName(name), accessor(accessor), description(description) {}
		Variable(std::string name, double *source, std::string description) : 
			Variable(name, std::bind([&] { return *source; }), description) {};

		double get() const { return accessor(); }
		std::string getS() const;
		std::string name;
		std::string originalName;
		std::string description;
	private:
		std::function<double()> accessor;
	};

	class Statistics
	{
	public:
		Statistics();
		~Statistics();
		void addVariable(const Variable &var);

		void enableLogging(bool enable = true) { loggingActive = enable; }
		void log();
		void init(void *ini);
	private:
		std::vector<Variable> variables;
		bool loggingActive;
		std::fstream outputStream;

		struct c_{
			std::string outputFilename;
			std::string delimiter;
		} config;
	};

};


extern MM::Statistics statistics;
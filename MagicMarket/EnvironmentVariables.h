#pragma once

#include <map>
#include <string>

namespace MM
{
class EnvironmentVariables
{
	struct Var
	{
		enum class Condition
		{
			None = 0,
			ExpertActive = 1,
		} condition;
		
		std::string get();

		Var *condThen, *condElse;

		Var() : condition(Condition::None), condThen(nullptr), condElse(nullptr) {}
		Var(std::string s) : Var() { constValue = s; }
		~Var();
		void assignValue(std::string value);
	private:
		std::string constValue = "";
	};
public:
	EnvironmentVariables();
	~EnvironmentVariables();
	void init(void *ini);
	std::string replace(const std::string &s);
	void set(const std::string &name, const std::string &value);
	std::string get(const std::string &name);
private:
	std::map<std::string, EnvironmentVariables::Var> vars;
};

} // namespace MM

extern MM::EnvironmentVariables environmentVariables;
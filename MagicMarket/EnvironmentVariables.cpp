#include "EnvironmentVariables.h"
#include "Market.h"
#include "ExpertAdvisor.h"

#include <SimpleIni.h>

#include <iostream>

MM::EnvironmentVariables environmentVariables;

namespace MM
{

EnvironmentVariables::Var::~Var()
{
	delete condThen;
	delete condElse;
}

std::string EnvironmentVariables::Var::get()
{
	switch (this->condition)
	{
	case Condition::None:
		return constValue;
	case Condition::ExpertActive:
	{
		for (ExpertAdvisor *expert : market.getExperts())
		{
			if (expert->getName() != constValue) continue;
			delete condElse;
			new (this) Var(std::move(*condThen));
			return this->get();
		}
		delete condThen;
		new (this) Var(std::move(*condElse));
		return this->get();
	}
	default:
		assert(false);
	}
	return "";
}

std::tuple<std::string::size_type, std::string::size_type> matchParenthesis(std::string &s, std::string::size_type beginPos)
{
	std::string::size_type first = s.find('(', beginPos);
	int state = 0;
	for (std::string::size_type i = first + 1; i < s.size(); ++i)
	{
		if (s[i] == '(') ++state;
		else if (s[i] == ')')
		{
			if (state != 0) --state;
			else
			{
				return std::make_tuple(first, i);
			}
		}
	}
	return std::make_tuple(std::string::npos, std::string::npos);
}

void EnvironmentVariables::Var::assignValue(std::string value)
{
	// Check for conditional value
	std::string::size_type i = value.find("?");
	if (i != std::string::npos)
	{
		std::string::size_type thenBegin, thenEnd;
		std::string::size_type elseBegin, elseEnd;
		std::tie(thenBegin, thenEnd) = matchParenthesis(value, i + 1);
		if (thenBegin != std::string::npos)
		{
			std::tie(elseBegin, elseEnd) = matchParenthesis(value, thenEnd + 1);
		}

		if (thenBegin == std::string::npos || elseBegin == std::string::npos)
		{
			std::cout << "Error assigning variable (got '" << value << "')" << std::endl;
			return;
		}

		const std::string conditionString = value.substr(0, i);
		const std::string thenString = value.substr(thenBegin + 1, (thenEnd - thenBegin) - 1);
		const std::string elseString = value.substr(elseBegin + 1, (elseEnd - elseBegin) - 1);
		
		condThen = new Var();
		condThen->assignValue(thenString);
		condElse = new Var();
		condElse->assignValue(elseString);

		const std::string expertActivePrefix = "Expert::";
		if (conditionString.find(expertActivePrefix) == 0)
		{
			condition = Condition::ExpertActive;
			constValue = conditionString.substr(expertActivePrefix.size());
		}
		return;
	}

	// Otherwise, just assign a constant value.
	constValue = value;
}

EnvironmentVariables::EnvironmentVariables()
{
}


EnvironmentVariables::~EnvironmentVariables()
{
}

void EnvironmentVariables::init(void *_ini)
{
	const CSimpleIniA &ini = *(CSimpleIniA*)_ini;

	for (size_t i = 1; i < 99; ++i)
	{
		const std::string varname = "Environment Variable " + std::to_string(i);
		const std::string name = ini.GetValue(varname.c_str(), "Name", "");
		const std::string value = ini.GetValue(varname.c_str(), "Value", "");
		if (name.empty()) break;

		vars.emplace(name, std::move(Var()));
		vars.at(name).assignValue(value);
	}
}

void EnvironmentVariables::set(const std::string &name, const std::string &value)
{
	vars[name] = value;
}

std::string EnvironmentVariables::get(const std::string &name)
{
	if (!vars.count(name)) return "";
	return vars[name].get();
}

std::string EnvironmentVariables::replace(const std::string &old)
{
	if (old.find('{') == std::string::npos) return old;
	std::string replaced(old);
	for (std::pair<const std::string, EnvironmentVariables::Var> &pair : vars)
	{
		const std::string name = "{" + pair.first + "}";
		const std::string::size_type i = replaced.find(name);
		if (i == std::string::npos) continue;
		replaced.replace(i, name.size(), pair.second.get());
	}
	return replaced;
}

} // namespace MM
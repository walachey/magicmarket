#pragma once
#include <ql/types.hpp>

namespace MM
{

	class Account
	{
	public:
		Account();
		~Account();

		QuantLib::Decimal leverage;
		QuantLib::Decimal balance;
		QuantLib::Decimal margin;
		QuantLib::Decimal marginFree;
	};

};
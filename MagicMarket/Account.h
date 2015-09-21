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

		void update(QuantLib::Decimal leverage, QuantLib::Decimal balance, QuantLib::Decimal margin, QuantLib::Decimal marginFree)
		{
			this->leverage = leverage;
			this->balance = balance;
			this->margin = margin;
			this->marginFree = marginFree;
		}
	};

};
#pragma once

#include <stdint.h>
#include <ql/types.hpp>
#include <string>


namespace MM
{
	class Tick;

	class Trade
	{
	public:
		Trade();
		~Trade();

		std::string currencyPair;
		QuantLib::Decimal orderPrice;
		QuantLib::Decimal takeProfitPrice;
		QuantLib::Decimal stopLossPrice;
		QuantLib::Decimal lotSize;

		int32_t ticketID;

		enum Type {
			T_BUY,
			T_SELL
		} type;

		QuantLib::Decimal getProfitAtTick(const Tick& tick) const;

		// completes the trade data with saved information
		std::string getSaveFileName();
		void load();
		void save();
		void removeSaveFile();

		static Trade Buy(std::string currencyPair, QuantLib::Decimal lotSize)
		{
			Trade trade;
			trade.currencyPair = currencyPair;
			trade.type = T_BUY;
			trade.lotSize = lotSize;
			return trade;
		}

		static Trade Sell(std::string currencyPair, QuantLib::Decimal lotSize)
		{
			Trade trade = Trade::Buy(currencyPair, lotSize);
			trade.type = T_SELL;
			return trade;
		}
	};

};
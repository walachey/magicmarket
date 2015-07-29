#include "RSI.h"
#include "Moves.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{

		RSI::RSI(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
			rsi   = std::numeric_limits<double>::quiet_NaN();
			moves = static_cast<Moves*>(Moves(currencyPair, history, seconds).init());
		}


		RSI::~RSI()
		{
		}

		void RSI::declareExports() const
		{
			exportVariable("RSI", getRSI, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void RSI::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			const double &UMA = moves->getUpMA();
			const double &DMA = moves->getDownMA();
			if (std::isnan(UMA) || std::isnan(DMA)) return;

			if (DMA == 0.0)
			{
				if (UMA == 0.0)
					rsi = 50.0;
				else
					rsi = 100.0;
				return;
			}

			const double RS = UMA / DMA;
			rsi = 100.0 - 100.0 / (1.0 + RS);
		}
	};
};
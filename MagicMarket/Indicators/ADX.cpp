#include "ADX.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

#include "Helpers.h"

#include "Moves.h"
#include "ATR.h"

namespace MM
{
	namespace Indicators
	{
		void ADX::reset()
		{
			pDIMA = pDIMA_pushed = std::numeric_limits<double>::quiet_NaN();
			mDIMA = mDIMA_pushed = std::numeric_limits<double>::quiet_NaN();
			adx = std::numeric_limits<double>::quiet_NaN();
			lastPushedMA = 0;
		}

		ADX::ADX(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
			moves = Indicators::get<Moves>(currencyPair, history, seconds);
			atr = Indicators::get<ATR>(currencyPair, history, seconds);
		}

		ADX::~ADX()
		{
		}

		void ADX::declareExports() const
		{
			exportVariable("pDIMA", std::bind(&ADX::getpDIMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("mDIMA", std::bind(&ADX::getmDIMA, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
			exportVariable("ADX", std::bind(&ADX::getADX, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void ADX::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			const double pDI = 100.0 * moves->getPlusDMMA()  / atr->getATRMA();
			const double mDI = 100.0 * moves->getMinusDMMA() / atr->getATRMA();

			pDIMA = Math::MA(pDIMA_pushed, pDI, history);
			mDIMA = Math::MA(mDIMA_pushed, mDI, history);

			if (time > (lastPushedMA + seconds))
			{
				lastPushedMA = time;
				pDIMA_pushed = pDIMA;
				mDIMA_pushed = mDIMA;
			}

			adx = 100.0 * std::abs(pDIMA - mDIMA) / (pDI + mDI);
		}
	};
};
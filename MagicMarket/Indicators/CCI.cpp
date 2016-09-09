#include "CCI.h"
#include "Moves.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{
		void CCI::OnlineMeanAbsoluteDeviation::reset()
		{
			n = 0;
			mean = 0.0;
			M2 = 4.0;
			MAD = 0.0;
		}

		void CCI::OnlineMeanAbsoluteDeviation::update(double newValue)
		{
			n += 1;
			const double delta = newValue - mean;
			mean += delta / static_cast<double>(n);
			double newDelta = newValue - mean;
			M2 += std::sqrt(std::abs(delta * newDelta));
			MAD = M2 / static_cast<double>(n);
		}

		void CCI::reset()
		{
			cci = std::numeric_limits<double>::quiet_NaN();
			typicalPriceMA = std::numeric_limits<double>::quiet_NaN();
			lastUpdateTime = 0;
			OnlineMAD.reset();
		}

		CCI::CCI(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
		}


		CCI::~CCI()
		{
		}

		void CCI::declareExports() const
		{
			exportVariable("CCI", std::bind(&CCI::getCCI, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void CCI::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			if (secondsSinceStart < seconds) return;
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;

			MM::TimePeriod now = stock->getTimePeriod(time - seconds, time);
			const MM::PossibleDecimal high = now.getHigh();
			const MM::PossibleDecimal low = now.getLow();
			const MM::PossibleDecimal close = now.getClose();

			if (!high.get() || !low.get()) return;

			const QuantLib::Decimal typicalPrice = (*high + *low + *close) / 3.0;
			const bool doUpdate = (lastUpdateTime == 0) || (time > lastUpdateTime + seconds);

			if (doUpdate)
			{
				lastUpdateTime = time;
				OnlineMAD.update(typicalPrice);
				typicalPriceMA = Math::MA(typicalPriceMA, typicalPrice, history);
			}

			cci = (1.0 / 0.0015) * (typicalPrice - typicalPriceMA) / OnlineMAD.MAD;
			assert(std::isnormal(cci) || cci == 0.0);
			assert(cci >= -10000.0 && cci <= +10000.0);
		}
	};
};
#include "KRI.h"
#include "SMA.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{
		void KRI::reset()
		{
			kri = std::numeric_limits<double>::quiet_NaN();
		}

		KRI::KRI(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
			sma = Indicators::get<SMA>(currencyPair, history, seconds);
		}

		KRI::~KRI()
		{
		}

		void KRI::declareExports() const
		{
			exportVariable("KRI", std::bind(&KRI::getKRI, this), "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void KRI::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			const double &smaValue = sma->getSMA();
			if (std::isnan(smaValue)) return;

			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;
			MM::TimePeriod now = stock->getTimePeriod(time - seconds, time);
			const PossibleDecimal price = now.getClose();
			if (!price.get()) return;

			kri = 100.0 * (*price - smaValue) / smaValue;
		}
	};
};
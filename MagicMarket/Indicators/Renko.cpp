#include "Renko.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{
		void Renko::reset()
		{
			lastBarAt = std::numeric_limits<double>::quiet_NaN();
			currentBarIndex = 0;

			bars.clear();
			for (int i = 0; i < history; ++i)
				bars.emplace_back(std::numeric_limits<double>::quiet_NaN());
		}

		Renko::Renko(std::string currencyPair, int history, double minimumChange) :
			currencyPair(currencyPair),
			history(history),
			minimumChange(minimumChange)
		{
		}

		Renko::~Renko()
		{
		}

		void Renko::declareExports() const
		{
			exportVariable("RenkoBar", std::bind(&Renko::getCurrentDirection, this), "min.change " + std::to_string(minimumChange) + ", memory " + std::to_string(history));
		}

		void Renko::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			Stock *stock = market.getStock(currencyPair);
			if (stock == nullptr) return;
			MM::TimePeriod now = stock->getTimePeriod(time);
			const PossibleDecimal price = now.getClose();
			if (!price.get()) return;

			if (std::isnan(lastBarAt))
			{
				lastBarAt = *price;
				return;
			}

			// Do we need a new bar?
			const double change = *price - lastBarAt;
			const double absChange = std::abs(change);
			if (absChange < minimumChange) return;

			// Upwards or downwards bar?
			double direction = 0.0;
			if (change > 0.0) direction = +1.0;
			else direction = -1.0;

			bars[currentBarIndex] = direction;
			currentBarIndex = (currentBarIndex + 1) % bars.size();
		}

		int Renko::getOffsetIndex(int offset) const
		{
			offset += currentBarIndex - 1;
			while (offset < 0) offset += bars.size();
			offset = offset % bars.size();
			return offset;
		}

		double Renko::getCurrentDirection() const
		{
			const double value = bars[getOffsetIndex(0)];
			if (std::isnan(value)) return 0.0;
			return value;
		}

		std::vector<double> Renko::getBars(int n) const
		{
			std::vector<double> subbars;
			subbars.resize(n);

			for (int i = 0; i < n; ++i)
			{
				const double value = bars[getOffsetIndex(-i)];
				if (std::isnan(value)) return {};
				subbars[n - i - 1] = value;
			}
			return subbars;
		}
	};
};
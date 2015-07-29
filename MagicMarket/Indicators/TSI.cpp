#include "TSI.h"
#include "SMA.h"
#include "Moves.h"
#include "Market.h"
#include "Stock.h"
#include "TimePeriod.h"

namespace MM
{
	namespace Indicators
	{

		TSI::TSI(std::string currencyPair, int history, int seconds) :
			currencyPair(currencyPair),
			history(history),
			seconds(seconds)
		{
			tsi = std::numeric_limits<double>::quiet_NaN();
			momentumDoubleMA    = std::numeric_limits<double>::quiet_NaN();
			absMomentumDoubleMA = std::numeric_limits<double>::quiet_NaN();
			moves = static_cast<Moves*>(Moves(currencyPair, 2 * history, seconds).init());
		}

		void TSI::declareExports() const
		{
			exportVariable("TSI", getTSI, "period " + std::to_string(seconds) + ", memory " + std::to_string(history));
		}

		void TSI::update(const std::time_t &secondsSinceStart, const std::time_t &time)
		{
			const double &momentumMA    = moves->getMomentumMA();
			const double &absMomentumMA = moves->getAbsoluteMomentumMA();
			if (std::isnan(momentumMA) || std::isnan(absMomentumMA)) return;

			momentumDoubleMA    = Math::MA2(momentumDoubleMA, momentumMA, history);
			absMomentumDoubleMA = Math::MA2(absMomentumDoubleMA, absMomentumMA, history);
			
			tsi = 100.0 * momentumDoubleMA / absMomentumDoubleMA;
		}
	};
};
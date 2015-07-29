#pragma once

#include "Base.h"
#include <string>

namespace MM
{
	class Stock;

	namespace Indicators
	{
		class SMA;

		// http://fxcodebase.com/wiki/index.php/Kairi_Relative_Index_%28KRI%29
		class KRI : public Base
		{
		public:
			KRI(std::string currencyPair, int history, int seconds);
			virtual ~KRI();

			virtual void declareExports() const;
			virtual void update(const std::time_t &secondsSinceStart, const std::time_t &time) override;
			virtual bool operator== (const Base &otherBase) const
			{
				const KRI* other = dynamic_cast<const KRI*>(&otherBase);
				if (other == nullptr) return false;
				return (other->history == history) && (other->seconds == seconds) && (other->currencyPair == currencyPair);
			}

			double getKRI() { return kri; }

		private:
			std::string currencyPair;
			int history;
			int seconds;
			double kri;

			SMA *sma;
		};

	};
};
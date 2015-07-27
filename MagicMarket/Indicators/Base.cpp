#include "Base.h"

#include "Market.h"
#include <vector>
#include <memory>

namespace MM
{
	namespace Indicators
	{
		Base::Base()
		{
		}


		Base::~Base()
		{
		}

		Base *Base::init()
		{
			std::vector<Base*> &indicators = market.getIndicators();

			// check if an indicator with our configuration already exists
			for (Base * &indicator : indicators)
			{
				if (*indicator == *this) return indicator;
			}

			// register anew
			Base *singleton = static_cast<Base*>(malloc(sizeof(*this)));
			memcpy(singleton, this, sizeof(*this));
			indicators.push_back(singleton);
			return singleton;
		}
	};
};
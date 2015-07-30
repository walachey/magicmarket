#include "Base.h"

#include "Market.h"
#include <vector>
#include <memory>

namespace MM
{
	namespace Indicators
	{
		std::vector<Base*> &getActiveIndicators()
		{
			return market.getIndicators();
		}

		Base::Base()
		{
		}


		Base::~Base()
		{
		}
	};
};
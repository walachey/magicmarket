#include "Trade.h"

namespace MM
{

	Trade::Trade() : ticketID(-1), orderPrice(0.0), takeProfitPrice(0.0), stopLossPrice(0.0), lotSize(0.0)
	{
	}


	Trade::~Trade()
	{
	}

};
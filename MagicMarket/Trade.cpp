#include "Trade.h"
#include "Market.h"

#include <fstream>
#include <algorithm>

#include <filesystem>
namespace filesystem = std::tr2::sys;

namespace MM
{

	Trade::Trade() : ticketID(-1), orderPrice(0.0), takeProfitPrice(0.0), stopLossPrice(0.0), lotSize(0.0)
	{
	}


	Trade::~Trade()
	{
	}

	std::string Trade::getSaveFileName()
	{
		std::ostringstream os;
		os << market.getSaveFolderName() << "/trades";
		filesystem::path path(os.str());
		if (!filesystem::exists(path))
			filesystem::create_directory(path);
		os << "/" << ticketID << ".trade";
		return os.str();
	}

	void Trade::save()
	{
		std::fstream file(getSaveFileName().c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
		if (!file.good()) return;

		int32_t version(1);
		file.write((char*)&version, sizeof(version));
		file.write((char*)&stopLossPrice, sizeof(stopLossPrice));
		file.write((char*)&takeProfitPrice, sizeof(takeProfitPrice));
	}

	void Trade::load()
	{
		
		std::fstream file(getSaveFileName().c_str(), std::ios_base::in | std::ios_base::binary);
		if (!file.good()) return;

		int32_t version(0);
		file.read((char*)&version, sizeof(version));

		QuantLib::Decimal sl(0.0), tp(0.0);

		if (version == 1)
		{
			file.read((char*)&sl, sizeof(sl));
			file.read((char*)&tp, sizeof(tp));
		}

		if (sl == 0.0) sl = stopLossPrice;
		if (tp == 0.0) tp = takeProfitPrice;

			
		if (stopLossPrice == 0.0)
			stopLossPrice = sl;
		else
		{
			if (type == T_BUY) stopLossPrice = std::max(sl, stopLossPrice);
			else stopLossPrice = std::min(sl, stopLossPrice);
		}
			
		if (takeProfitPrice == 0.0)
			takeProfitPrice = tp;
		else
		{
			if (type == T_BUY) takeProfitPrice = std::min(tp, takeProfitPrice);
			else takeProfitPrice = std::max(tp, takeProfitPrice);
		}
		
	}
};
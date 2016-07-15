#include "MTInterface.h"
#include "UDP.h"

#include "Market.h"
#include "VirtualMarket.h"
#include "Trade.h"

#include <iostream>
#include <WinSock2.h>
#include <SimpleIni.h>
#include <cassert>

Interface::MetaTrader::MTInterface metatrader;

namespace Interface
{
	namespace MetaTrader
	{
		MTInterface::MTInterface()
		{

		}

		MTInterface::~MTInterface()
		{

		}

		void MTInterface::init(void *_ini)
		{
			const CSimpleIniA &ini = *(CSimpleIniA*)_ini;
			const bool virtualMarketEnabled = ini.GetBoolValue("Virtual Market", "Enabled", false);

			// Nothing to do if the virtual simulation is used.
			if (virtualMarketEnabled) return;

			this->port = ini.GetLongValue("Metatrader Interface", "Port", 4301);

			setup();
		}

		void MTInterface::setup()
		{
			session = std::make_unique<::Interface::Internet::WSASession>();
			socket = std::make_unique<::Interface::Internet::UDPSocket>();
			socket->bind(this->port);
		}

		void MTInterface::checkIncomingMessages()
		{
			do
			{
				const std::tuple<SOCKADDR_IN*, std::string*> message = this->socket->recv();
				const std::string &data = *std::get<1>(message);
				if (data.size() == 0) break;
				assert(data.size() > 8);
				char * const dataPointer = const_cast<char*>((&data[0]));
				const int32_t &type = *reinterpret_cast<int32_t*>(dataPointer);
				void * const messageContentsPointer = dataPointer + sizeof(int32_t);

				switch (type)
				{
				case MetaTrader::Message::Type::bridgeTick:
					{
						MetaTrader::Message::Tick &msg = *reinterpret_cast<MetaTrader::Message::Tick*>(messageContentsPointer);
						market.onNewTickMessageReceived(msg.pair, msg.bid, msg.ask, msg.timestamp);
					}
					break;
				case MetaTrader::Message::Type::bridgeAccountInfo:
					{
						MetaTrader::Message::AccountInfo &msg = *reinterpret_cast<MetaTrader::Message::AccountInfo*>(messageContentsPointer);
						market.account.update(msg.leverage, msg.balance, msg.margin, msg.freeMargin);
					}
					break;
				case MetaTrader::Message::Type::bridgeOrders:
					{
						const size_t oneOrderLength = 7 * sizeof(char) + 2 * sizeof(int32_t) + 3 * sizeof(double) + 2 * sizeof(int32_t) + 2 * sizeof(double);
						assert(oneOrderLength == 56);
						if ((data.size() - sizeof(int32_t)) % oneOrderLength != 0)
						{
							assert(false);
							break;
						}
						const int numOrders = (data.size() - sizeof(int32_t)) / oneOrderLength;
						MetaTrader::Message::Order *order = reinterpret_cast<MetaTrader::Message::Order*> (messageContentsPointer);

						market.saveAndClearTrades();
						for (int i = 0; i < numOrders; ++i)
						{
							::MM::Trade *trade = new ::MM::Trade();
							trade->currencyPair = order->pair;
							trade->lotSize = order->lots;
							trade->orderPrice = order->openPrice;
							trade->setStopLossPrice(order->stopLoss);
							trade->setTakeProfitPrice(order->takeProfit);
							trade->ticketID = order->tickedID;

							if (order->type == 0) trade->type = ::MM::Trade::T_BUY;
							else trade->type = ::MM::Trade::T_SELL;

							market.onNewTradeMessageReceived(trade);
						}
					}
					break;
				case MetaTrader::Message::Type::bridgeUp:
					{
						MetaTrader::Message::BridgeUp &msg = *reinterpret_cast<MetaTrader::Message::BridgeUp*>(messageContentsPointer);
						std::cout << "Bridge connected for " << msg.pair << " @ timestamp " << msg.timestamp << std::endl;
					}
					break;
				case MetaTrader::Message::Type::bridgeDown:
					{
						MetaTrader::Message::BridgeDown &msg = *reinterpret_cast<MetaTrader::Message::BridgeDown*>(messageContentsPointer);
						std::cout << "Bridge disconnected for " << msg.pair << " @ timestamp " << msg.timestamp << std::endl;
					}
					break;
				case MetaTrader::Message::Type::bridgeError:
					{
						char *message = reinterpret_cast<char*>(messageContentsPointer) + sizeof(int32_t);
						std::cout << "Bridge error: " << message << std::endl;
					}
					break;
				default:
					assert(false);
					break;
				};

			} while (true);
		}

		void MTInterface::send(std::string data, int probabilityToSendInVirtualMode)
		{
			if (market.isVirtual())
			{
				//virtualMarket->proxySend(data);
				if (probabilityToSendInVirtualMode == 0) return;
				if (probabilityToSendInVirtualMode < 100)
				{
					int randomDraw = rand() % 100;
					if (randomDraw > probabilityToSendInVirtualMode) return;
				}

				if (virtualMarket->isSilent) return;
			}

			assert(false);
		}
	};
};
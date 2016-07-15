#pragma once

#include <memory>

namespace Interface
{
	namespace Internet
	{
		class WSASession;
		class UDPSocket;
	};

	namespace MetaTrader
	{
		namespace Message
		{
			enum Type
			{
				bridgeUp = 1,
				bridgeDown,
				bridgeTick,
				bridgeAccountInfo,
				bridgeOrders,
				bridgeError,
			};
#pragma pack(push, 1)
			struct BridgeUp
			{
				char pair[7];
				int32_t timestamp;
			};

			struct BridgeDown
			{
				char pair[7];
				int32_t timestamp;
			};

			static_assert(sizeof(double) == 8, "Require double to be 64bit length.");
			struct Tick
			{
				char pair[7];
				double bid;
				double ask;
				int32_t timestamp;
			};

			struct AccountInfo
			{
				double leverage;
				double balance;
				double margin;
				double freeMargin;
			};

			struct Order
			{
				char pair[7];
				int32_t type;
				int32_t tickedID;
				double openPrice;
				double takeProfit;
				double stopLoss;
				int32_t timestampOpen;
				int32_t timestampExpire;
				double lots;
				double profit;
			};
#pragma pack(pop)
		};
		
		class MTInterface
		{
		public:
			MTInterface();
			~MTInterface();

			void init(void *ini);
			void checkIncomingMessages();

		private:
			void setup();

			std::unique_ptr<::Interface::Internet::WSASession> session;
			std::unique_ptr<::Interface::Internet::UDPSocket> socket;
			int port;
		};
	};
};

extern Interface::MetaTrader::MTInterface metatrader;

// MetaTraderBridge.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

extern "C"
{
#define DLL_EXPORT __declspec(dllexport)

#define MM_BRIDGE_UP 1
#define MM_BRIDGE_DOWN 2
#define MM_BRIDGE_TICK 3
#define MM_BRIDGE_ACCOUNTINFO 4
#define MM_BRIDGE_ORDERS 5
#define MM_BRIDGE_ERROR 6
#define MM_NEW_ORDER 7
#define MM_CLOSE_ORDER 8
#define MM_UPDATE_ORDER 9

	int errorCode = 0;
	int winsockInitialized = 0;
	void tryInitWinsock()
	{
		const bool alreadyDone = winsockInitialized > 0;
		if (alreadyDone)
		{
			++winsockInitialized;
			return;
		}

		WSADATA wsa;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			errorCode = 1;
			return;
		}
		++winsockInitialized;
	}

	void tryCleanup()
	{
		--winsockInitialized;
		if (winsockInitialized > 0) return;
		WSACleanup();
	}

	struct SocketLink
	{
		SOCKET socket;
		struct sockaddr_in recipient;
		char *currencyPair;
		struct commandData_
		{
			int ticketID;
			int orderType;
			double orderPrice;
			double takeProfitPrice;
			double stopLossPrice;
			double lotSize;
		} commandData;
	};

	DLL_EXPORT int WINAPI mm_cleanup(int _link)
	{
		SocketLink *link = (SocketLink*)_link;
		free(link->currencyPair);
		free(link);
		tryCleanup();
		return 1;
	}

	DLL_EXPORT int WINAPI mm_init(const char * server, int port, const char * currencyPair)
	{
		tryInitWinsock();
		if (winsockInitialized == 0) return 1;

		SOCKET sock;
		struct sockaddr_in si_other;

		if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			errorCode = 2;
			return 2;
		}

		u_long nonblocking = 1;
		const int setBlockingResult = ioctlsocket(sock, FIONBIO, &nonblocking);

		if (setBlockingResult != NO_ERROR)
		{
			return 3;
		}

		//setup address structure
		memset((char *)&si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(port);
		si_other.sin_addr.S_un.S_addr = inet_addr(server);
		
		SocketLink *link = (SocketLink*)malloc(sizeof(SocketLink));
		link->socket = sock;
		link->recipient = si_other;
		const int strlen_ = strlen(currencyPair) + 1;
		assert(strlen_ == fixedCurrencyPairLength);
		link->currencyPair = (char*)malloc(strlen_);
		memcpy(link->currencyPair, currencyPair, strlen_);
		
		return (int)link;
	}

	int sendPreparedMessage(int _link, char **message, int len)
	{
		int returnValue = 0;
		SocketLink *link = (SocketLink *)_link;
		if (sendto(link->socket, *message, len, 0, (struct sockaddr *) &link->recipient, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			errorCode = 3;
			returnValue = 1;
		}
		free(*message);
		*message = 0;
		return returnValue;
	}

	const int fixedCurrencyPairLength = 7;
#define PREPARE(TYPE, LEN) const char type = TYPE; int totalLength = sizeof(type) + LEN; char *__message = (char*)malloc(totalLength); int index = sizeof(type); memcpy((void*)__message, &type, sizeof(type));
#define PACKCURRENCY() {memcpy((void*)(__message + index), ((SocketLink*)(link))->currencyPair, fixedCurrencyPairLength); index += fixedCurrencyPairLength; };
#define PACKOTHER(X) {memcpy((void*)(__message + index), &X, sizeof(X)); index += sizeof(X); };
#define PACKSTR(X, LEN) {memcpy((void*)(__message + index), X, LEN); index += LEN; };
#define SEND() sendPreparedMessage(link, &__message, totalLength);

	DLL_EXPORT int WINAPI mm_sendUp(int link, int timestamp)
	{
		PREPARE(MM_BRIDGE_UP, fixedCurrencyPairLength + sizeof(timestamp));
		PACKCURRENCY();
		PACKOTHER(timestamp);
		SEND();
		return 0;
	}

	DLL_EXPORT int WINAPI mm_sendDown(int link, int timestamp)
	{
		PREPARE(MM_BRIDGE_DOWN, fixedCurrencyPairLength + sizeof(timestamp));
		PACKCURRENCY();
		PACKOTHER(timestamp);
		SEND();
		return 0;
	}

	DLL_EXPORT int WINAPI mm_sendTick(int link, double bid, double ask, int timestamp)
	{
		PREPARE(MM_BRIDGE_TICK, fixedCurrencyPairLength + sizeof(bid) + sizeof(ask) + sizeof(timestamp));
		PACKCURRENCY();
		PACKOTHER(bid);
		PACKOTHER(ask);
		PACKOTHER(timestamp);
		return SEND();
	}

	DLL_EXPORT int WINAPI mm_sendAccountInfo(int link, double leverage, double balance, double margin, double freeMargin)
	{
		PREPARE(MM_BRIDGE_ACCOUNTINFO, sizeof(leverage) + sizeof(balance) + sizeof(margin) + sizeof(freeMargin));
		PACKOTHER(leverage);
		PACKOTHER(balance);
		PACKOTHER(margin);
		PACKOTHER(freeMargin);
		SEND();
		return 0;
	}

	DLL_EXPORT int WINAPI mm_sendError(int link, const char *message)
	{
		const int len = strlen(message);
		PREPARE(MM_BRIDGE_ERROR, sizeof(len) + len);
		PACKOTHER(len);
		PACKSTR(message, len);
		SEND();
		return 0;
	}

	typedef struct {
		char * orderBatch;
		int orderBatchCurrentSize;
		int orderBatchCurrentIndex;
		SocketLink *server_link;
	} OrderBatchData;

	DLL_EXPORT int WINAPI mm_beginOrderBatch(int link)
	{
		OrderBatchData *data = (OrderBatchData*)malloc(sizeof(OrderBatchData));

		
		data->orderBatchCurrentSize = 512;
		data->orderBatch = (char*)malloc(data->orderBatchCurrentSize);
		const char type = MM_BRIDGE_ORDERS;
		memcpy((void*)data->orderBatch, &type, sizeof(type));
		data->orderBatchCurrentIndex = sizeof(type);
		data->server_link = (SocketLink*)link;
		return (int)data;
	}

	DLL_EXPORT int WINAPI mm_addOrder(int batch, int type, int ticketID, double openPrice, double takeProfit, double stopLoss, int timestampOpen, int timestampExpire, double lots, double profit)
	{
		OrderBatchData *data = (OrderBatchData*)batch;

		const int requiredSize = fixedCurrencyPairLength + sizeof(type) + sizeof(ticketID) + sizeof(openPrice) + sizeof(takeProfit) + sizeof(stopLoss) + sizeof(timestampOpen) + sizeof(timestampExpire) + sizeof(lots) + sizeof(profit);

		const int openSize = data->orderBatchCurrentSize - data->orderBatchCurrentIndex;

		if (openSize < requiredSize)
		{
			const int newSize = data->orderBatchCurrentSize + requiredSize;
			void * reallocatedMemory = realloc((void*)data->orderBatch, newSize);

			// if reallocation failed, just allocate a new block
			if (reallocatedMemory == NULL)
			{
				reallocatedMemory = malloc(newSize);
				memcpy(reallocatedMemory, (void*)data->orderBatch, data->orderBatchCurrentSize);
				free(data->orderBatch);
				data->orderBatch = (char*)reallocatedMemory;
			}
			else
			{
				// this should not be necessary
				data->orderBatch = (char*)reallocatedMemory;
			}
		}

#define PACK(X) {memcpy((void*)(data->orderBatch + data->orderBatchCurrentIndex), &X, sizeof(X)); data->orderBatchCurrentIndex += sizeof(X);};
		memcpy((void*)(data->orderBatch + data->orderBatchCurrentIndex), data->server_link->currencyPair, fixedCurrencyPairLength);
		data->orderBatchCurrentIndex += fixedCurrencyPairLength;
		PACK(type);
		PACK(ticketID);
		PACK(openPrice);
		PACK(takeProfit);
		PACK(stopLoss);
		PACK(timestampOpen);
		PACK(timestampExpire);
		PACK(lots);
		PACK(profit);
#undef PACK
		return 0;
	}

	DLL_EXPORT int WINAPI mm_sendOrderBatch(int __link, int batch)
	{
		SocketLink *link = (SocketLink*)__link;
		OrderBatchData *data = (OrderBatchData*)batch;
		if (sendto(link->socket, data->orderBatch, data->orderBatchCurrentIndex + 1, 0, (struct sockaddr *) &link->recipient, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			errorCode = 3;
		}
		free(data->orderBatch);
		free(data);
		return 0;
	}

#define BEGINPACKING(X) char * _currentPosition = (char*)&X;
#define UNPACK(X) {memcpy(&X, _currentPosition, sizeof(X)); _currentPosition += sizeof(X);};
	const int RECVBUFFERLENGTH = 2048;
	char RECVBUFFER[RECVBUFFERLENGTH];

	DLL_EXPORT int WINAPI mm_checkCommand(int __link)
	{
		SocketLink *link = (SocketLink*)__link;
		int senderAddressLength = sizeof(struct sockaddr_in);
		const int recvResults = recvfrom(link->socket, RECVBUFFER, RECVBUFFERLENGTH - 1, 0, (struct sockaddr *) &link->recipient, &senderAddressLength);

		if (recvResults == SOCKET_ERROR)
		{
			const int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)
			{
				return 0;
			}
			return -1;
		}
		// Safety - not really necessary at this point, though.
		RECVBUFFER[recvResults] = '\0';

		BEGINPACKING(RECVBUFFER);
		int messageType; UNPACK(messageType);

		switch (messageType)
		{
		case MM_NEW_ORDER:
			{
				UNPACK(link->commandData.orderType);
				UNPACK(link->commandData.orderPrice);
				UNPACK(link->commandData.takeProfitPrice);
				UNPACK(link->commandData.stopLossPrice);
				UNPACK(link->commandData.lotSize);
			}
			break;
		case MM_CLOSE_ORDER:
			{
				UNPACK(link->commandData.ticketID);
			}
			break;
		case MM_UPDATE_ORDER:
			{
				UNPACK(link->commandData.ticketID);
				UNPACK(link->commandData.takeProfitPrice);
				UNPACK(link->commandData.stopLossPrice);
			}
			break;
		};

		return messageType;
	}

	DLL_EXPORT int WINAPI mm_receiveTicketID(int link) { return ((SocketLink*)link)->commandData.ticketID; }
	DLL_EXPORT int WINAPI mm_receiveOrderType(int link) { return ((SocketLink*)link)->commandData.orderType; }
	DLL_EXPORT double WINAPI mm_receiveOrderPrice(int link) { return ((SocketLink*)link)->commandData.orderPrice; }
	DLL_EXPORT double WINAPI mm_receiveTakeProfitPrice(int link) { return ((SocketLink*)link)->commandData.takeProfitPrice; }
	DLL_EXPORT double WINAPI mm_receiveStopLossPrice(int link) { return ((SocketLink*)link)->commandData.stopLossPrice; }
	DLL_EXPORT double WINAPI mm_receiveLotSize(int link) { return ((SocketLink*)link)->commandData.lotSize; }

#undef BEGINPACKING
#undef UNPACK
};
#pragma once

#include <memory>
#include <string>
#include <tuple>

#include <WinSock2.h>

struct WSAData;

namespace Interface
{
	namespace Internet
	{
		class WSASession
		{
		public:
			WSASession();
			~WSASession();
		private:
			std::unique_ptr<WSAData> data;
		};

		class UDPSocketReplyChannel;

		class UDPSocket
		{
		public:
			UDPSocket();
			~UDPSocket();

			void bind(unsigned short port);
			void send(const std::string& address, unsigned short port, const char* buffer, int len, int flags = 0);
			void send(sockaddr_in& address, const char* buffer, int len, int flags = 0);
			std::tuple<struct sockaddr_in*, std::string*> recv();
			bool recv(struct sockaddr_in *sender, char* buffer, int &len, int flags = 0);

			UDPSocketReplyChannel getReplyChannel();
		private:
			SOCKET socket;
			std::unique_ptr<std::string> localBuffer;
			std::unique_ptr<struct sockaddr_in> lastSender;

			friend class UDPSocketReplyChannel;
		};

		class UDPSocketReplyChannel
		{
		public:
			UDPSocketReplyChannel() : socket(nullptr) {}
			UDPSocketReplyChannel(UDPSocket &source) : socket(&source), sourceAddress(*source.lastSender.get()) {}
			void send(const char* buffer, int len, int flags = 0)
			{
				if (this->socket == nullptr) return;
				return this->socket->send(this->sourceAddress, buffer, len, flags);
			}
		private:
			UDPSocket *socket;
			struct sockaddr_in sourceAddress;
		};
	}
};
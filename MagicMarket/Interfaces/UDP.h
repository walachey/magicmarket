#pragma once

#include <memory>
#include <string>
#include <tuple>

struct WSAData;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
typedef UINT_PTR        SOCKET;
struct sockaddr_in;

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
		private:
			SOCKET socket;
			std::unique_ptr<std::string> localBuffer;
			std::unique_ptr<struct sockaddr_in> lastSender;
		};
	}
};
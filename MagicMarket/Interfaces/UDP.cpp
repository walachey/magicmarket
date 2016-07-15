#include "UDP.h"

#include <WinSock2.h>
#include <system_error>
#include <cassert>

namespace Interface
{
	namespace Internet
	{
		WSASession::WSASession()
		{
			data = std::make_unique<WSAData>();
			int ret = WSAStartup(MAKEWORD(2, 2), data.get());
			if (ret != 0)
				throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup Failed");
		}

		WSASession::~WSASession()
		{
			WSACleanup();
		}

		UDPSocket::UDPSocket()
		{
			this->socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (this->socket == INVALID_SOCKET)
				throw std::system_error(WSAGetLastError(), std::system_category(), "Error opening socket");
		}

		UDPSocket::~UDPSocket()
		{
			closesocket(this->socket);
		}

		void UDPSocket::send(const std::string& address, unsigned short port, const char* buffer, int len, int flags)
		{
			sockaddr_in add;
			add.sin_family = AF_INET;
			add.sin_addr.s_addr = inet_addr(address.c_str());
			add.sin_port = htons(port);
			return send(add, buffer, len, flags);
		}

		void UDPSocket::send(sockaddr_in& address, const char* buffer, int len, int flags)
		{
			int ret = sendto(this->socket, buffer, len, flags, reinterpret_cast<SOCKADDR *>(&address), sizeof(address));
			if (ret < 0)
				throw std::system_error(WSAGetLastError(), std::system_category(), "sendto failed");
		}

		std::tuple<struct sockaddr_in*, std::string*> UDPSocket::recv()
		{
			if (localBuffer.get() == nullptr)
			{
				localBuffer = std::make_unique<std::string>(2048, '\0');
				lastSender = std::make_unique<SOCKADDR_IN>();
			}
			int len = localBuffer.get()->capacity();
			const char *oldMemoryLocation = &localBuffer.get()->at(0);
			recv(lastSender.get(), &localBuffer.get()->at(0), len);
			localBuffer->resize(len);
			assert(oldMemoryLocation == &localBuffer.get()->at(0));
			return std::make_tuple(lastSender.get(), localBuffer.get());
		}

		bool UDPSocket::recv(SOCKADDR_IN *sender, char* buffer, int &len, int flags)
		{
			int size = sizeof(SOCKADDR_IN);
			const int ret = recvfrom(this->socket, buffer, len - 1, flags, reinterpret_cast<SOCKADDR *>(sender), &size);
			if (ret == WSAEWOULDBLOCK)
			{
				buffer[0] = '\0';
				len = 0;
				return false;
			}
			if (ret < 0)
				throw std::system_error(WSAGetLastError(), std::system_category(), "recvfrom failed");
			assert(ret < len);
			// make the buffer zero terminated
			buffer[ret] = '\0';
			len = ret;
			return true;
		}

		void UDPSocket::bind(unsigned short port)
		{
			sockaddr_in add;
			add.sin_family = AF_INET;
			add.sin_addr.s_addr = htonl(INADDR_ANY);
			add.sin_port = htons(port);

			const bool True = true;
			setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &True, sizeof(True));

			u_long blocking = 0;
			const int setBlockingResult = ioctlsocket(this->socket, FIONBIO, &blocking);
			if (setBlockingResult != NO_ERROR)
			{
				throw std::system_error(setBlockingResult, std::system_category(), "ioctlsocket failed with error: " + std::to_string(setBlockingResult));
			}

			int ret = ::bind(this->socket, reinterpret_cast<SOCKADDR *>(&add), sizeof(add));
			if (ret < 0)
				throw std::system_error(WSAGetLastError(), std::system_category(), "Bind failed");
		}
	}
};
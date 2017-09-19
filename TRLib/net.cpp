#include <cstdlib>
#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "net.h"

namespace tr {
	namespace net {
		int ctrler::init() {
			WSADATA wsaData;
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			switch (result) {
				case 0: return NET_GOOD;
				case WSASYSNOTREADY: return NET_NOT_READY;
				case WSAVERNOTSUPPORTED: return NET_NOT_SUPPORTED;
				case WSAEINPROGRESS: return NET_BLOCKING_OPERATION;
				case WSAEPROCLIM: return NET_LIMIT;
				case WSAEFAULT: return NET_FAULT;
				default: return NET_UNKNOWN;
			}
		}

		int ctrler::cleanup() {
			int result = WSACleanup();
			switch (result) {
				case 0: return NET_GOOD;
				case WSANOTINITIALISED: return NET_GOOD;
				case WSAENETDOWN: return NET_NOT_READY;
				case WSAEINPROGRESS: return NET_BLOCKING_OPERATION;
				default: return NET_UNKNOWN;
			}
		}

		ctrler::client *ctrler::newClient(addrinfo *info) {
			return new ctrler::client(info);
		}

		ctrler::client::client(addrinfo *info) {
			this->info = info;
		}

		int ctrler::client::connect() {
			SOCKET conSock = INVALID_SOCKET;
			for (; info != NULL; info = info->ai_next) {
				conSock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
				if (conSock == INVALID_SOCKET) {
					return -1;
				}

				int result = ::connect(conSock, info->ai_addr, (int)info->ai_addrlen);
				if (result == SOCKET_ERROR) {
					closesocket(conSock);
					conSock = INVALID_SOCKET;
					continue;
				}
				break;
			}

			if (conSock == INVALID_SOCKET) {
				return -1;
			}

			return 0;
		}
	}
}
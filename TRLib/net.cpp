#include <cstdlib>
#include <cstdint>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "net.h"

namespace tr {
	namespace net {
		int ctrler::init() {
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult) {
				lastSystemError = iResult;
				return 1;
			}
			return 0;
		}

		int ctrler::cleanup() {
			int iResult = WSACleanup();
			if (iResult) {
				lastSystemError = iResult;
				return 1;
			}
			return 0;
		}

		int ctrler::getLastSystemError() {
			return lastSystemError;
		}

		int ctrler::translateSystemError(int systemErr) {
			switch (systemErr) {
				case 0: return NET_GOOD;
				case WSANOTINITIALISED: return NET_NOT_INIT;
				case WSAENETDOWN: return NET_NOT_READY;
				case WSAEINPROGRESS: return NET_BLOCKING_OPERATION;
				case WSASYSNOTREADY: return NET_NOT_READY;
				case WSAVERNOTSUPPORTED: return NET_NOT_SUPPORTED;
				case WSAEPROCLIM: return NET_LIMIT;
				case WSAEFAULT: return NET_FAULT;
				case WSATRY_AGAIN: return NET_TRY_AGAIN;
				case WSAEINVAL: return NET_INVAL_ARG;
				case WSANO_RECOVERY: return NET_NO_RECOVERY;
				case WSAEAFNOSUPPORT: return NET_AF_NO_SUPPORT;
				case WSA_NOT_ENOUGH_MEMORY: return NET_NOT_ENOUGH_MEMORY;
				case WSAHOST_NOT_FOUND: return NET_HOST_NOT_FOUND;
				case WSATYPE_NOT_FOUND: return NET_TYPE_NOT_FOUND;
				case WSAESOCKTNOSUPPORT: return NET_SOCK_TYPE_NO_SUPPORT;
				case WSANO_DATA: return NET_NO_DATA;
				case WSAEADDRINUSE: return NET_ADDR_IN_USE;
				case WSAEINTR: return NET_OP_INTR;
				case WSAEALREADY: return NET_ALREADY;
				case WSAEADDRNOTAVAIL: return NET_ADDR_NOT_AVAIL;
				case WSAECONNREFUSED: return NET_CONN_REFUSED;
				case WSAEISCONN: return NET_IS_CONN;
				case WSAENETUNREACH: return NET_NET_UNREACH;
				case WSAEHOSTUNREACH: return NET_HOST_UNREACH;
				case WSAENOBUFS: return NET_NO_BUFS;
				case WSAENOTSOCK: return NET_NOT_SOCK;
				case WSAETIMEDOUT: return NET_TIMEDOUT;
				case WSAEWOULDBLOCK: return NET_WOULD_BLOCK;
				case WSAEACCES: return NET_NO_ACCES;
				case WSAEMFILE: return NET_TOO_MANY_SOCK;
				case WSAEINVALIDPROVIDER: return NET_INVALID_PROVIDER;
				case WSAEINVALIDPROCTABLE: return NET_INVALID_PROCTABLE;
				case WSAEPROTONOSUPPORT: return NET_PROTO_NO_SUPPORT;
				case WSAEPROTOTYPE: return NET_WRONG_PROTO_TYPE;
				case WSAEPROVIDERFAILEDINIT: return NET_PROVIDER_INIT_FAILED;

				default: return NET_UNKNOWN;
			}
		}

		int ctrler::getLastError() {
			return translateSystemError(getLastSystemError());
		}

		ctrler::client *ctrler::newClient() {
			return new ctrler::client(this);
		}

		ctrler::server *ctrler::newServer() {
			return new ctrler::server(this);
		}

		ctrler::client::client(ctrler *baseCtrler) {
			this->baseCtrler = baseCtrler;
		}

		ctrler *ctrler::client::getCtrler() {
			return baseCtrler;
		}

		int ctrler::client::connect(char *address, char *port) {
			addrinfo *result = nullptr, hints, *ptr = nullptr;
			SOCKET connectSocket;
			int iResult;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (iResult = getaddrinfo(address, port, &hints, &result)) {
				baseCtrler->lastSystemError = iResult;
				return 1;
			}

			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
				connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (connectSocket == INVALID_SOCKET) {
					baseCtrler->lastSystemError = WSAGetLastError();
					return 1;
				}

				if (::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
					baseCtrler->lastSystemError = WSAGetLastError();
					closesocket(connectSocket);
					connectSocket = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (connectSocket == INVALID_SOCKET) {
				baseCtrler->lastSystemError = WSAGetLastError();
				return 1;
			}

			conSock = connectSocket;
			return 0;
		}

		int ctrler::client::connect(char *address, uint16_t port) {
			char s_port[6];
			sprintf_s(s_port, 5, "%uh", port);
			return connect(address, s_port);
		}

		int ctrler::client::close() {
			if (closesocket(conSock)) {
				baseCtrler->lastSystemError = WSAGetLastError();
				return 1;
			}

			return 0;
		}

		ctrler::server::server(ctrler *baseCtrler) {
			this->baseCtrler = baseCtrler;
		}

		ctrler *ctrler::server::getCtrler() {
			return baseCtrler;
		}

		int ctrler::server::listen(char *port) {
			
		}

		int ctrler::server::listen(uint16_t port) {
			char s_port[6];
			sprintf_s(s_port, 5, "%uh", port);
			return listen(s_port);
		}

		SOCKET ctrler::server::accept() {

		}

		int ctrler::server::close() {
			if (closesocket(listenSock)) {
				baseCtrler->lastSystemError = WSAGetLastError();
				return 1;
			}

			return 0;
		}
	}
}
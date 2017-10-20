#include <cstdlib>
#include <cstdint>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

#include "net.h"

namespace tr {
	namespace net {
		int ctrler::init() {
			lastSystemError = 0;
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
			}
			return 0;
		}

		int ctrler::cleanup() {
			int iResult = WSACleanup();
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
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
				case WSAENOTCONN: return NET_NOT_CONNECTED;
				case WSAENETRESET: return NET_RESET;
				case WSAEOPNOTSUPP: return NET_OP_NOT_SUPPORTED;
				case WSAESHUTDOWN: return NET_SHUTDOWN;
				case WSAEMSGSIZE: return NET_TOO_LONG;
				case WSAECONNABORTED: return NET_CONN_ABORTED;
				case WSAECONNRESET: return NET_CONN_RESET;

				default: return NET_UNKNOWN;
			}
		}

		int ctrler::getLastError() {
			return translateSystemError(getLastSystemError());
		}

		void ctrler::setLastSystemError(int systemErr) {
			lastSystemError = systemErr;
		}

		ctrler::client *ctrler::newClient() {
			return new ctrler::client();
		}

		ctrler::server *ctrler::newServer() {
			return new ctrler::server();
		}

		ctrler::client::client() { }

		void ctrler::client::setLastSystemError(int systemErr) {
			lastSystemError = systemErr;
			ctrler::setLastSystemError(systemErr);
		}

		int ctrler::client::getLastSystemError() {
			return lastSystemError;
		}

		int ctrler::client::getLastError() {
			return translateSystemError(lastSystemError);
		}

		ctrler::socket *ctrler::client::connect(char *address, char *port) {
			addrinfo *result = nullptr, hints, *ptr = nullptr;
			SOCKET connectSocket;
			int iResult;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (iResult = getaddrinfo(address, port, &hints, &result)) {
				setLastSystemError(iResult);
				return nullptr;
			}

			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
				connectSocket = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (connectSocket == INVALID_SOCKET) {
					setLastSystemError(WSAGetLastError());
					return nullptr;
				}

				if (::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
					setLastSystemError(WSAGetLastError());
					closesocket(connectSocket);
					connectSocket = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (connectSocket == INVALID_SOCKET) {
				setLastSystemError(WSAGetLastError());
				return nullptr;
			}

			return new socket(connectSocket);
		}

		ctrler::socket *ctrler::client::connect(char *address, uint16_t port) {
			char s_port[6];
			sprintf_s(s_port, 5, "%uh", port);
			return connect(address, s_port);
		}

		ctrler::server::server() { }

		void ctrler::server::setLastSystemError(int systemErr) {
			lastSystemError = systemErr;
			ctrler::setLastSystemError(systemErr);
		}

		int ctrler::server::getLastSystemError() {
			return lastSystemError;
		}

		int ctrler::server::getLastError() {
			return translateSystemError(lastSystemError);
		}

		int ctrler::server::listen(char *port) {
			addrinfo *result = nullptr, hints;
			SOCKET listenSocket;
			int iResult;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			iResult = getaddrinfo(NULL, port, &hints, &result);
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
			}
			
			listenSocket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (listenSocket == INVALID_SOCKET) {
				setLastSystemError(WSAGetLastError());
				freeaddrinfo(result);
				return -1;
			}

			if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
				setLastSystemError(WSAGetLastError());
				freeaddrinfo(result);
				closesocket(listenSocket);
				return -1;
			}

			freeaddrinfo(result);

			if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
				setLastSystemError(WSAGetLastError());
				closesocket(listenSocket);
				return -1;
			}

			listenSock = listenSocket;

			return 0;
		}

		int ctrler::server::listen(uint16_t port) {
			char s_port[6];
			sprintf_s(s_port, 5, "%uh", port);
			return listen(s_port);
		}

		ctrler::socket *ctrler::server::accept() {
			SOCKET clientSocket = ::accept(listenSock, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				setLastSystemError(WSAGetLastError());
				closesocket(listenSock);
				return nullptr;
			}

			return new socket(clientSocket);
		}

		int ctrler::server::close() {
			if (closesocket(listenSock)) {
				ctrler::lastSystemError = WSAGetLastError();
				return -1;
			}

			return 0;
		}

		ctrler::socket::socket(SOCKET sock) {
			this->conSock = sock;
		}

		void ctrler::socket::setLastSystemError(int systemErr) {
			lastSystemError = systemErr;
			ctrler::setLastSystemError(systemErr);
		}

		int ctrler::socket::getLastSystemError() {
			return lastSystemError;
		}

		int ctrler::socket::getLastError() {
			return translateSystemError(lastSystemError);
		}

		size_t ctrler::socket::write(const char *buf, size_t offset, size_t len) {
			if (len == NET_ERROR) len -= 1;
			const char *baseBuf = buf + offset;
			size_t wrote = 0;
			while (wrote < len) {
				int result = send(conSock, baseBuf + wrote, (int)(len - wrote), NULL);
				if (result == SOCKET_ERROR) {
					setLastSystemError(WSAGetLastError());
					return NET_ERROR;
				}
				wrote += result;
			}

			return wrote;
		}

		size_t ctrler::socket::read(char *buf, size_t offset, size_t len) {
			if (len == NET_ERROR) len -= 1;
			char *baseBuf = buf + offset;
			size_t read = 0;
			while (read < len) {
				int result = recv(conSock, baseBuf + read, (int)(len - read), MSG_WAITALL);
				if (result == SOCKET_ERROR) {
					setLastSystemError(WSAGetLastError());
					return NET_ERROR;
				}
				read += result;
			}

			return read;
		}
	}
}
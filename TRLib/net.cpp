#include <cstdlib>
#include <cstdint>
#include <stdio.h>
#include <limits>
#if defined(_WIN32)
#include <string>
#elif defined(__unix__) || defined(__APPLE__)
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "net.h"

namespace tr {
	namespace net {
		int ctrler::lastSystemError = 0;

		int ctrler::init() {
#ifdef _WIN32
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
			}
#endif
			return 0;
		}

		int ctrler::cleanup() {
#ifdef _WIN32
			int iResult = WSACleanup();
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
			}
#endif
			return 0;
		}

		int ctrler::getLastSystemError() {
			return lastSystemError;
		}

		int ctrler::translateSystemError(int systemErr) {
			switch (systemErr) {
				case 0: return NET_GOOD;
#ifdef _WIN32
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
#elif defined(__unix__) || defined(__APPLE__)

#endif

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

		ctrler::socket *ctrler::client::connect(char *address, char *port, bool endianNegotiation) {
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
					setLastSystemError(SYSTEM_ERROR);
					return nullptr;
				}

				if (::connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
					setLastSystemError(SYSTEM_ERROR);
					CLOSE_SOCKET(connectSocket);
					connectSocket = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (connectSocket == INVALID_SOCKET) {
				setLastSystemError(SYSTEM_ERROR);
				return nullptr;
			}

			socket *sock = new socket(connectSocket);
			if (endianNegotiation) {
				sock->write((const char *)&magicNumber, 0, sizeof(magicNumber));
				sock->read((char *)&sock->nativeEndian, 0, sizeof(sock->nativeEndian));
			}
			return sock;
		}

		ctrler::socket *ctrler::client::connect(char *address, uint16_t port, bool endianNegotiation) {
			char s_port[6];
			snprintf(s_port, 6, "%hu", port);
			return connect(address, s_port, endianNegotiation);
		}

		ctrler::server::server() { }

		ctrler::server::~server() {
			close();
		}

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

		int ctrler::server::listen(char *port, char *address) {
			addrinfo *result = nullptr, hints;
			SOCKET listenSocket;
			int iResult;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			iResult = getaddrinfo(address, port, &hints, &result);
			if (iResult) {
				setLastSystemError(iResult);
				return -1;
			}
			
			listenSocket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (listenSocket == INVALID_SOCKET) {
				setLastSystemError(SYSTEM_ERROR);
				freeaddrinfo(result);
				return -1;
			}

			if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
				setLastSystemError(SYSTEM_ERROR);
				freeaddrinfo(result);
				CLOSE_SOCKET(listenSocket);
				return -1;
			}

			freeaddrinfo(result);

			if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
				setLastSystemError(SYSTEM_ERROR);
				CLOSE_SOCKET(listenSocket);
				return -1;
			}

			listenSock = listenSocket;

			return 0;
		}

		int ctrler::server::listen(uint16_t port, char *address) {
			char s_port[6];
			snprintf(s_port, 6, "%hu", port);
			return listen(s_port, address);
		}

		ctrler::socket *ctrler::server::accept(bool endianNegotiation) {
			SOCKET clientSocket = ::accept(listenSock, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				setLastSystemError(SYSTEM_ERROR);
				CLOSE_SOCKET(listenSock);
				return nullptr;
			}

			socket *sock = new socket(clientSocket);
			if (endianNegotiation) {
				uint32_t clientMagicNumber;
				if (sock->read((char *)&clientMagicNumber, 0, sizeof(clientMagicNumber)) < sizeof(clientMagicNumber)) {
					setLastSystemError(sock->getLastSystemError());
					delete sock;
					return nullptr;
				}
				sock->nativeEndian = clientMagicNumber == magicNumber;
				if (sock->write((char *)&sock->nativeEndian, 0, sizeof(sock->nativeEndian)) < sizeof(sock->nativeEndian)) {
					setLastSystemError(sock->getLastSystemError());
					delete sock;
					return nullptr;
				}
			}
			return sock;
		}

		int ctrler::server::close() {
			if (CLOSE_SOCKET(listenSock)) {
				ctrler::lastSystemError = SYSTEM_ERROR;
				return -1;
			}

			return 0;
		}

		ctrler::socket::socket(SOCKET sock) {
			this->conSock = sock;
		}

		ctrler::socket::~socket() {
			close();
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

		bool ctrler::socket::getNativeEndian() {
			return nativeEndian;
		}

		size_t ctrler::socket::write(const char *buf, size_t offset, size_t len) {
			const char *baseBuf = buf + offset;
			size_t wrote = 0;
			while (wrote < len) {
				int result = send(conSock, baseBuf + wrote, (int)(len - wrote), 0);
				if (result == SOCKET_ERROR) {
					setLastSystemError(SYSTEM_ERROR);
					return wrote;
				}
				wrote += result;
			}

			return wrote;
		}

		size_t ctrler::socket::read(char *buf, size_t offset, size_t len) {
			char *baseBuf = buf + offset;
			size_t read = 0;
			while (read < len) {
				int result = recv(conSock, baseBuf + read, (int)(len - read), MSG_WAITALL);
				if (result == SOCKET_ERROR) {
					setLastSystemError(SYSTEM_ERROR);
					return read;
				}
				read += result;
			}

			return read;
		}

		int ctrler::socket::close() {
			return CLOSE_SOCKET(conSock);
		}
	}
}
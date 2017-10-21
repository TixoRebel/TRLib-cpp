#pragma once

#include <cstdint>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#define CLOSE_SOCKET(socket) closesocket(socket)
#define SYSTEM_ERROR WSAGetLastError()
#elif defined(__unix__) || defined(__APPLE__)
#define CLOSE_SOCKET(socket) ::close(socket)
#define SYSTEM_ERROR errno
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#else
#error Unsupported Platform
#endif


#include "neterr.h"

namespace tr {
	namespace net {
		class ctrler {
			ctrler(const ctrler& that) = delete;

			static const uint32_t magicNumber = 67305985;
			static int lastSystemError;
			static void setLastSystemError(int systemErr);

			public:
				class socket {
					friend class ctrler;
					socket(const socket& that) = delete;

					private:
						SOCKET conSock = INVALID_SOCKET;
						int lastSystemError = NET_GOOD;
						bool nativeEndian = false;
						socket(SOCKET sock);
						void setLastSystemError(int systemErr);
					public:
						~socket();
						int getLastSystemError();
						int getLastError();
						bool getNativeEndian();
						size_t write(const char *buf, size_t offset, size_t len);
						size_t read(char *buf, size_t offset, size_t len);
						int close();
				};

				class client {
					friend class ctrler;
					client(const client& that) = delete;

					private:
						int lastSystemError = NET_GOOD;
						client();
						void setLastSystemError(int systemErr);
					public:
						int getLastSystemError();
						int getLastError();
						socket *connect(char *address, char *port, bool endianNegotiation = true);
						socket *connect(char *address, uint16_t port, bool endianNegotiation = true);
				};

				class server {
					friend class ctrler;
					server(const server& that) = delete;

					private:
						SOCKET listenSock = INVALID_SOCKET;
						int lastSystemError = NET_GOOD;
						server();
						void setLastSystemError(int systemErr);
					public:
						~server();
						int getLastSystemError();
						int getLastError();
						int listen(char *port);
						int listen(uint16_t port);
						socket *accept(bool endianNegotiation = true);
						int close();
				};
				
				static int init();
				static int cleanup();
				static int getLastSystemError();
				static int translateSystemError(int systemErr);
				static int getLastError();
				static ctrler::client *newClient();
				static ctrler::server *newServer();
		};
	}
}
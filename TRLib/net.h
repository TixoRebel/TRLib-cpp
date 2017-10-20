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

			static int lastSystemError;
			static void setLastSystemError(int systemErr);

			public:
				class socket {
					friend class ctrler;
					socket(const socket& that) = delete;

					private:
						SOCKET conSock = INVALID_SOCKET;
						int lastSystemError = NET_GOOD;
						socket(SOCKET sock);
						void setLastSystemError(int systemErr);
					public:
						int getLastSystemError();
						int getLastError();
						size_t write(const char *buf, size_t offset, size_t len);
						size_t read(char *buf, size_t offset, size_t len);
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
						socket *connect(char *address, char *port);
						socket *connect(char *address, uint16_t port);
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
						int getLastSystemError();
						int getLastError();
						int listen(char *port);
						int listen(uint16_t port);
						socket *accept();
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
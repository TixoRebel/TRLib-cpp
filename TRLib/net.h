#pragma once

#include <cstdlib>
#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include "neterr.h"

namespace tr {
	namespace net {
		class ctrler {
			class client {
				friend class ctrler;
				private:
					ctrler *baseCtrler;
					SOCKET conSock = INVALID_SOCKET;
					client(ctrler *baseCtrler);
				public:
					ctrler *getCtrler();
					int connect(char *address, char *port);
					int connect(char *address, uint16_t port);
					int close();
			};

			class server {
				friend class ctrler;
				private:
					ctrler *baseCtrler;
					SOCKET listenSock = INVALID_SOCKET;
					server(ctrler *baseCtrler);
				public:
					ctrler *getCtrler();
					int listen(char *port);
					int listen(uint16_t port);
					SOCKET accept();
					int close();
			};

			int lastSystemError = NET_GOOD;

			public:
				int init();
				int cleanup();
				int getLastSystemError();
				int translateSystemError(int systemErr);
				int getLastError();
				ctrler::client *newClient();
				ctrler::server *newServer();
		};
	}
}
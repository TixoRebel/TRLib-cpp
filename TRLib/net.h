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
					addrinfo *info;
					client(addrinfo *info);
				public:
					int connect();
			};

			public:
				int init();
				int cleanup();
				ctrler::client *newClient(addrinfo *info);
		};
	}
}
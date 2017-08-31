#pragma once

#include <event2/util.h>

class Client {
public:
	Client(evutil_socket_t sock, struct event_base *b) : 
		socket(sock), base(b) {
	}

	int read(void *buf, size_t len);
	int write(const void *buf, size_t len);

	evutil_socket_t socket;     // client socket
	struct event_base *base; // bufferevent created on the socket
};

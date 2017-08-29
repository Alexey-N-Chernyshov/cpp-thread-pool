#pragma once

#include <event2/util.h>

class Client {
public:
	Client(evutil_socket_t sock, struct bufferevent *buffer_event) : 
		socket(sock), buf_ev(buffer_event) {
	}

	int read(void *buf, size_t len);
	int write(const void *buf, size_t len);

private:
	evutil_socket_t socket;     // client socket
	struct bufferevent *buf_ev; // bufferevent created on the socket
};

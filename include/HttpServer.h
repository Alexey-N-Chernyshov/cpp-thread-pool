#pragma once

#include <event2/listener.h>
#include <event2/bufferevent.h>

#include "thread_pool/Future.h"
#include "thread_pool/System.h"

using namespace me::eax::thread_pool;

class HttpServer {
public:
	HttpServer();
	~HttpServer();

	int listen(const char *host, int port);

	HttpServer(HttpServer const&) = delete;
	void operator=(HttpServer const&) = delete;

private:
	struct event_base *base;
	struct evconnlistener *listener;
	System system;

	static void accept_connection_cb(struct evconnlistener *listener,
			evutil_socket_t fd, struct sockaddr *addr, int sock_len,
			void *arg);
	static void accept_error_cb(struct evconnlistener *listener, void *arg);
	static void read_cb(struct bufferevent *buf_ev, void *arg);
	static void write_cb(struct bufferevent *buf_ev, void *arg);
	static void event_cb(struct bufferevent *buf_ev, short events, void *arg);
};

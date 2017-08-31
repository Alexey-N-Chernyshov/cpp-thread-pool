#include <HttpServer.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include <event2/buffer.h>

#include "Client.h"

#define MAX_WORKERS 12

HttpServer::HttpServer() {
}

HttpServer::~HttpServer() {
}

int HttpServer::listen(const char *host, int port) {
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	std::cout << "HttpServer is running on " << host << ":" << port << std::endl;

	// run event base for connections in mutliple threads
	for (int i = 0; i < MAX_WORKERS; i++) {
		struct event_base *thread_base = event_base_new();
		Client *client = new Client(-1, thread_base);
		workers.push_back(client);
		struct bufferevent *bev = bufferevent_socket_new(thread_base, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(bev, read_cb, NULL, event_cb, client); // dummy event
		bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
		system.spawn([thread_base]() -> int {
			std::cout << "thread start" << std::endl;
			int res = event_base_dispatch(thread_base);
			event_base_free(thread_base);
			std::cout << "thread end" << std::endl;
			return res;
		});
	}
	
	base = event_base_new();
	if (base == NULL) {
		perror("event_base_new");
		return -1;
	}

	listener = evconnlistener_new_bind(base, accept_connection_cb, this,
			(LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE),
			-1, (struct sockaddr *)&sin, sizeof(sin)
		);
	if (listener == NULL) {
		perror("evconnlistener_new_bind");
		return -1;
	}

	evconnlistener_set_error_cb(listener, accept_error_cb);

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_base_free(base);

	return 0;
}

void HttpServer::accept_connection_cb(struct evconnlistener *listener,
		evutil_socket_t sock, struct sockaddr *addr, int sock_len, void *arg) {
	(void)sock;
	(void)addr;     // unused parameter
	(void)sock_len; // unused parameter
	(void)listener; // unused parameter
	HttpServer *server = (HttpServer *)arg;
	
	std::cout << "new connection" << std::endl;

	static int counter = 0;
	counter++;
	Client *worker = server->workers[counter % MAX_WORKERS];
	struct event_base *base = worker->base;
	struct bufferevent *buf_ev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(buf_ev, read_cb, write_cb, event_cb, worker);
	bufferevent_enable(buf_ev, (EV_READ | EV_WRITE));
}

void HttpServer::accept_error_cb(struct evconnlistener *listener, void *arg) {
	(void)arg; // unused parameter

	struct event_base *base = evconnlistener_get_base(listener);
	int error = EVUTIL_SOCKET_ERROR();
	std::cerr << "Error " << error << "(" 
		<< evutil_socket_error_to_string(error) << ") in listener."
		<< std::endl;
	event_base_loopexit(base, NULL);
}

void HttpServer::read_cb(struct bufferevent *buf_ev, void *arg) {
	(void)arg;

	std::cout << "read_cb" << std::endl;

	char inbuf[256];
	struct evbuffer *buf_input = bufferevent_get_input(buf_ev);
	evbuffer_remove(buf_input, inbuf, 256);

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n";
	ss << "Content-Type: text/html\r\n";
	ss << "Content-Length: 2\r\n";
	ss << "Connection: keep-alive\r\n";
	ss << "\r\n";
	ss << "OK";

	struct evbuffer *buf_output = bufferevent_get_output(buf_ev);
	evbuffer_add(buf_output, ss.str().c_str(), ss.str().size());
}

void HttpServer::write_cb(struct bufferevent *buf_ev, void *arg) {
	(void)buf_ev; // unused parameter
	(void)arg;    // unused parameter
	
	std::cout << "write_cb" << std::endl;
}

void HttpServer::event_cb(struct bufferevent *buf_ev, short events, void *arg) {
	(void)buf_ev; // unused parameter
	(void)events; // unused parameter
	(void)arg;

	std::cout << "event_cb" << std::endl;

	if (events & BEV_EVENT_ERROR)
		std::cerr << "bufferevent error" << std::endl;
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
		std::cout << "connection close" << std::endl;
	}
}

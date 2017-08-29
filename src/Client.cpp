#include <Client.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>

int Client::read(void *data, size_t len) {
	struct evbuffer *buf_input = bufferevent_get_input(buf_ev);
	return evbuffer_remove(buf_input, data, len);
}

int Client::write(const void *data, size_t len) {
	struct evbuffer *buf_output = bufferevent_get_output(buf_ev);
	return evbuffer_add(buf_output, data, len);
}


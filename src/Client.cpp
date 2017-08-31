#include <Client.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>

int Client::read(void *data, size_t len) {
	(void)data;
	(void)len;
	return 0;
}

int Client::write(const void *data, size_t len) {
	(void)data;
	(void)len;
	return 0;
}


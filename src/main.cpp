#include <HttpServer.h>

auto main() -> int {
	HttpServer server;
	server.listen("127.0.0.1", 12345);
	return 0;
}

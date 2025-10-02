#include "HttpWebSocket.h"

engine::http::WebSocketConnection::WebSocketConnection(string Url)
{
	using namespace websocketpp;

	client<config::core_client> c;

	std::error_code errs;

	auto connection = c.get_connection(Url, errs);

	c.connect(connection);

	c.set_message_handler([](auto a, auto b)
	{
		a;
	});
}

#include "HttpWebSocket.h"
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
#include <Core/Log.h>

engine::http::WebSocketConnection::WebSocketConnection(string Url)
{
	ix::initNetSystem();
	ix::WebSocket* Socket = new ix::WebSocket();

	Socket->setUrl(Url);

	Socket->setOnMessageCallback([](const ix::WebSocketMessagePtr& Message) {
		switch (Message->type)
		{
		case ix::WebSocketMessageType::Message:
		{
			Log::Info(Message->str);
			break;
		}
		case ix::WebSocketMessageType::Open:
		{
			Log::Info("Connection opened.");
			break;
		}
		}
		Log::Info(std::to_string(int(Message->type)));
	});

	Socket->start();
}

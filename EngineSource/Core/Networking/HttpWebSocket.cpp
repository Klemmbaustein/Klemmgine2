#ifdef KLEMMGINE_WEBSOCKETS
#include "HttpWebSocket.h"
#include <Core/Platform/CodeAnalysis.h>
CODE_ANALYSIS_BEGIN_EXTERNAL_HEADER
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
CODE_ANALYSIS_END_EXTERNAL_HEADER
#include <Core/Log.h>

engine::http::WebSocketConnection::WebSocketConnection(string Url)
{
	ix::initNetSystem();
	Socket = new ix::WebSocket();
	Socket->setUrl(Url);

	Socket->setOnMessageCallback([this](const ix::WebSocketMessagePtr& Message) {
		switch (Message->type)
		{
		case ix::WebSocketMessageType::Message:
		{
			auto EngineMessage = WebSocketMessage{
				.Type = Message->binary ? WebSocketMessageType::Binary : WebSocketMessageType::Text,
				.Data = new BufferStream((uByte*)Message->str.data(), Message->str.size()),
			};
			if (OnMessage)
				OnMessage(EngineMessage);
			delete EngineMessage.Data;
			break;
		}
		case ix::WebSocketMessageType::Open:
		{
			if (OnOpened)
				OnOpened();
			break;
		}
		case ix::WebSocketMessageType::Close:
		{
			if (OnClosed)
				OnClosed();
			break;
		}
		case ix::WebSocketMessageType::Error:
		{
			Log::Error(Message->errorInfo.reason);
			break;
		}
		}
	});

	Socket->start();
}

engine::http::WebSocketConnection::~WebSocketConnection()
{
	Socket->stop();
	delete Socket;
}

void engine::http::WebSocketConnection::Send(const uByte* Data, size_t DataSize, bool IsBinary)
{
	if (IsBinary)
	{
		Socket->sendBinary(ix::IXWebSocketSendData((char*)Data, DataSize));
		return;
	}
	Socket->send(string((char*)Data, DataSize), IsBinary);
}
#endif
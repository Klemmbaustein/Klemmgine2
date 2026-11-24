#ifdef KLEMMGINE_WEBSOCKETS
#include "HttpWebSocket.h"
#include <tcpUtils/WebSocket.h>
#include <Core/Error/EngineError.h>
#include <Core/Log.h>

using namespace tcp;

engine::http::WebSocketConnection::WebSocketConnection(string Url)
{
	HttpRequest Request = HttpRequest(Url, "GET");
	Socket = new HttpWebSocket(&Request);

	Socket->receiveMessage = [this](tcp::WebSocketMessage Message) {
		auto EngineMessage = engine::http::WebSocketMessage{
			.Type = Message.operation == tcp::WebSocketOp::sendBinary
			? WebSocketMessageType::Binary : WebSocketMessageType::Text,
			.Data = new ReadOnlyBufferStream((uByte*)Message.payload, Message.payloadLength, false),
			.IsLast = Message.isLastChunk
		};
		if (OnMessage)
			OnMessage(EngineMessage);
		delete EngineMessage.Data;
	};

	Socket->receiveClose = [this](tcp::WebSocketCloseMessage Message) {
		if (Message.closeReason != WebSocketCloseReason::normalClosure)
		{
			Log::Error(string((char*)Message.payload, Message.payloadLength));
		}

		if (OnClosed)
			OnClosed();
	};
}

void engine::http::WebSocketConnection::Start()
{
	this->ConnectionThread = std::thread([this] {
		error::InitForThread("WebSocket listen thread");
		try
		{
			if (OnOpened)
			{
				OnOpened();
			}

			while (Socket->IsConnected())
			{
				Socket->ReadConnection();
			}
		}
		catch (NetworkException& e)
		{
			if (!IsClosed)
			{
				Log::Error(e.what());
				Socket->Close(WebSocketCloseReason::protocolError);
			}
		}
	});
}

engine::http::WebSocketConnection::~WebSocketConnection()
{
	IsClosed = true;
	try
	{
		if (Socket->IsConnected())
		{
			Socket->Close(WebSocketCloseReason::normalClosure);
		}
	}
	catch (NetworkException& e)
	{
		Log::Error(e.what());
	}
	ConnectionThread.join();
	delete Socket;
}

void engine::http::WebSocketConnection::Send(const uByte* Data, size_t DataSize, bool IsBinary)
{
	Socket->SendMessage(!IsBinary, (void*)Data, DataSize);
}
#endif
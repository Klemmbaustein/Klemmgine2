#ifdef KLEMMGINE_WEBSOCKETS
#pragma once
#include <Core/File/BinaryStream.h>
#include <optional>
#include <functional>

namespace ix
{
	class WebSocket;
}

namespace engine::http
{
	enum class WebSocketMessageType
	{
		Binary,
		Text,
	};

	struct WebSocketMessage
	{
		WebSocketMessageType Type = WebSocketMessageType::Binary;
		BufferStream* Data = nullptr;
	};

	class WebSocketConnection
	{
	public:
		WebSocketConnection(string Url);
		~WebSocketConnection();

		void Send(const uByte* Data, size_t DataSize, bool IsBinary = true);

		std::function<void()> OnOpened;
		std::function<void(const WebSocketMessage&)> OnMessage;
		std::function<void()> OnClosed;

	private:
		ix::WebSocket* Socket = nullptr;
	};
}
#endif
#ifdef KLEMMGINE_WEBSOCKETS
#pragma once
#include <Core/File/BinaryStream.h>
#include <optional>
#include <functional>
#include <thread>

namespace tcp
{
	class HttpWebSocket;
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
		ReadOnlyBufferStream* Data = nullptr;
		bool IsLast = true;
	};

	class WebSocketConnection
	{
	public:
		WebSocketConnection(string Url);
		~WebSocketConnection();

		void Start();

		void Send(const uByte* Data, size_t DataSize, bool IsBinary = true);

		std::function<void()> OnOpened;
		std::function<void(const WebSocketMessage&)> OnMessage;
		std::function<void()> OnClosed;

	private:
		tcp::HttpWebSocket* Socket = nullptr;
		std::thread ConnectionThread;
		bool IsClosed = false;
	};
}
#endif
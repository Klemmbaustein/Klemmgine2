#pragma once
#include <Core/File/BinaryStream.h>
#include <optional>

namespace engine::http
{
	struct WebSocketMessage
	{

	};

	class WebSocketConnection
	{
	public:
		WebSocketConnection(string Url);

		void Send(IBinaryStream* Data);

		std::optional<WebSocketMessage> Next();

	private:
		std::vector<WebSocketMessage> NewMessages;
	};
}
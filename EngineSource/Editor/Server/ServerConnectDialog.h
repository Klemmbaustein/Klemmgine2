#pragma once
#include "ServerConnection.h"

namespace engine::editor
{
	struct ConnectResult
	{
		bool Connect = false;
		ServerConnection* Connection = nullptr;
	};

	class ServerConnectDialog
	{
	public:
		static ConnectResult Show();
	};
}
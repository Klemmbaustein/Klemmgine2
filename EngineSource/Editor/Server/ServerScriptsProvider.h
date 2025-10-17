#pragma once
#include <Engine/Script/ScriptProvider.h>
#include "ServerConnection.h"

namespace engine::editor
{
	class ServerScriptProvider : public script::ScriptProvider
	{
	public:

		ServerScriptProvider(ServerConnection* Connection);

		std::vector<string> GetFiles() override;

		string GetFile(string Name) override;

		ServerConnection* Connection = nullptr;
	};
}
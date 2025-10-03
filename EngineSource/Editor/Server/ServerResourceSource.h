#pragma once
#include <Engine/File/Resource.h>
#include "ServerConnection.h"

namespace engine::editor
{
	class ServerResourceSource : public resource::ResourceSource
	{
	public:

		ServerResourceSource(ServerConnection* Connection);
		bool FileExists(string Path) override;
		ReadOnlyBufferStream* GetFile(string Path) override;

		std::map<string, string> GetFiles() override;

		ServerConnection* Connection = nullptr;
	};
}
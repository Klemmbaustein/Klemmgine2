#pragma once
#include "ScriptProvider.h"

namespace engine::script
{
	class FileScriptProvider : public ScriptProvider
	{
	public:
		std::vector<string> GetFiles() override;

		string GetFile(string Name) override;
	};
}
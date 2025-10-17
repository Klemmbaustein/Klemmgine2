#pragma once
#include <Core/Types.h>

namespace engine::script
{
	class ScriptProvider
	{
	public:
		virtual ~ScriptProvider() = default;

		virtual std::vector<string> GetFiles() = 0;

		virtual string GetFile(string Name) = 0;
	};
}
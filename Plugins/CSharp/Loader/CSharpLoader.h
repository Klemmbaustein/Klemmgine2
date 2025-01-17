#pragma once
#include <Engine/Types.h>

namespace engine::cSharp
{
	class CSharpLoader
	{
	public:
		virtual size_t CreateObjectInstance(size_t TypeId) = 0;
	};
}
#pragma once
#include <Core/Types.h>

namespace engine::cSharp
{
	class CSharpLoader
	{
	public:
		virtual size_t CreateObjectInstance(size_t TypeId) = 0;
		virtual void RemoveObjectInstance(size_t ObjId) = 0;
	};
}
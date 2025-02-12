#pragma once
#include <Core/Types.h>

namespace engine::cSharp
{
	class CSharpLoader
	{
	public:
		virtual ~CSharpLoader() {}

		virtual size_t CreateObjectInstance(size_t TypeId, void* NativeObject) = 0;
		virtual void RemoveObjectInstance(size_t ObjId) = 0;

		virtual void Update(float Delta) = 0;
	};
}
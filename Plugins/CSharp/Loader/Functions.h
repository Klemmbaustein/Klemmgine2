#pragma once

namespace engine::cSharp
{
	struct NativeFunction
	{
		const char* Name = nullptr;
		void* FunctionPointer = nullptr;
	};
}
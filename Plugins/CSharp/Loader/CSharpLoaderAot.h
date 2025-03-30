#pragma once
#include <Core/Types.h>
#include "CSharpLoader.h"
#include "Functions.h"
#include <Core/Platform/Platform.h>

namespace engine::cSharp
{
	class CSharpLoaderAot : public CSharpLoader
	{
	public:
		CSharpLoaderAot(const std::vector<NativeFunction>& Functions);
		~CSharpLoaderAot() override;

		size_t CreateObjectInstance(size_t TypeId, void* NativeObject) override;
		void RemoveObjectInstance(size_t ObjId) override;
		void Update(float Delta) override;

	private:
		using AotCreateObjectFn = size_t(*)(size_t ObjectType, void* Object);
		using AotDestroyObjectFn = void(*)(size_t ObjectId);
		using AotMainFn = void(*)(intptr_t ArrayPointer, uint32 ArrayLength);
		using AotUpdateFn = void(*)(float Delta);

		platform::SharedLibrary* AotLibrary = nullptr;

		AotDestroyObjectFn DestroyObject = nullptr;
		AotCreateObjectFn CreateObject = nullptr;
		AotUpdateFn UpdateAot = nullptr;
	};
}
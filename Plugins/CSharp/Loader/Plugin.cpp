#include <KlemmginePlugin.hpp>
#include "CSharpLoader.h"
#include "CSharpLoaderRuntime.h"
#include <Engine/Objects/SceneObject.h>

using namespace engine::cSharp;

CSharpLoader* Current = nullptr;

class CSharpObject : public engine::SceneObject
{
public:

	size_t CSharpTypeId;
	size_t CSharpObjectId = 0;

	CSharpObject(size_t CSharpTypeId)
	{
		this->CSharpTypeId = CSharpTypeId;
	}

	void Begin() override
	{
		CSharpObjectId = Current->CreateObjectInstance(CSharpTypeId);
	}

	void Update() override
	{
	}

	void OnDestroyed() override
	{
		Current->RemoveObjectInstance(CSharpObjectId);
	}
};

static void RegisterObject(const char* Name, size_t TypeId)
{
	engine::RegisterObject(Name, [TypeId]() -> engine::SceneObject*
		{
			return new CSharpObject(TypeId);
		});
}

static void LoadRuntime()
{
	engine::plugin::EnginePluginInterface* Interface = engine::plugin::GetInterface();

#undef STRUCT_MEMBER
#undef STRUCT_MEMBER_CALL_DIRECT
	// (void*)(size_t) cast to prevent GCC from complaining about casting function pointers to void*
#define STRUCT_MEMBER(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = (void*)(size_t)Interface->name},
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = (void*)(size_t)Interface->name},

	std::vector<NativeFunction> Functions = {
#include "../internal/InterfaceDefines.hpp"
	};

	Functions.push_back(NativeFunction{
		.Name = "RegisterCSharpObject",
		.FunctionPointer = (void*)&RegisterObject
		});

	Current = new CSharpLoaderRuntime(Functions);
}

ENGINE_EXPORT void RegisterTypes()
{
	LoadRuntime();
}

ENGINE_EXPORT void OnSceneLoaded(engine::Scene* New)
{
}
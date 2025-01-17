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

	CSharpObject(size_t CSharpTypeId)
	{
		this->CSharpTypeId = CSharpTypeId;
		Current->CreateObjectInstance(CSharpTypeId);
	}

	void Begin() override
	{
	}

	void Update() override
	{
	}

	void OnDestroyed() override
	{
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
#define STRUCT_MEMBER(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = Interface->name},
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = Interface->name},

	std::vector<NativeFunction> Functions = {
#include "../internal/InterfaceDefines.hpp"
	};

	Functions.push_back(NativeFunction{
		.Name = "RegisterCSharpObject",
		.FunctionPointer = &RegisterObject
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
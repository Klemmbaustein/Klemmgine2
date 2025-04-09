#include <KlemmginePlugin.hpp>
#include "CSharpLoader.h"
#include "CSharpLoaderRuntime.h"
#include "CSharpLoaderAot.h"
#include <Engine/Objects/SceneObject.h>

namespace engine::cSharp
{
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
			CSharpObjectId = Current->CreateObjectInstance(CSharpTypeId, this);
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

#define GET_SET_FN(name, type) \
static void SetObject ## name (SceneObject* Target, type name) {Target->name = name;} \
static type GetObject ## name (SceneObject* Target) {return Target->name;} \

#define STR(x) # x
#define REGISTER_GET_SET(fnName) \
Functions.push_back(NativeFunction{ \
	.Name = STR(SetObject ## fnName), \
	.FunctionPointer = (void*)&SetObject ## fnName \
}); \
Functions.push_back(NativeFunction{ \
	.Name = STR(GetObject ## fnName), \
	.FunctionPointer = (void*)&GetObject ## fnName \
});

	GET_SET_FN(Position, Vector3)
	GET_SET_FN(Rotation, Rotation3)
	GET_SET_FN(Scale, Vector3)

	static void LoadRuntime()
	{
		engine::plugin::EnginePluginInterface* Interface = engine::plugin::GetInterface();

#undef STRUCT_MEMBER
#undef STRUCT_MEMBER_CALL_DIRECT
		// (void*)(intptr_t) cast to prevent GCC from complaining about casting function pointers to void*
#define STRUCT_MEMBER(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = (void*)(intptr_t)Interface->name},
#define STRUCT_MEMBER_CALL_DIRECT(name, ret, args, func) NativeFunction{.Name = # name, .FunctionPointer = (void*)(intptr_t)Interface->name},

		std::vector<NativeFunction> Functions = {
#include "../internal/InterfaceDefines.hpp"
		};

		Functions.push_back(NativeFunction{
			.Name = "RegisterCSharpObject",
			.FunctionPointer = (void*)&RegisterObject
			});

		REGISTER_GET_SET(Position);
		REGISTER_GET_SET(Rotation);
		REGISTER_GET_SET(Scale);

		//Current = new CSharpLoaderRuntime(Functions);
		Current = new CSharpLoaderAot(Functions);
	}

}

ENGINE_EXPORT void RegisterTypes()
{
	engine::cSharp::LoadRuntime();
}

ENGINE_EXPORT void OnSceneLoaded(engine::Scene* New)
{
}

ENGINE_EXPORT void Update(float Delta)
{
	engine::cSharp::Current->Update(Delta);
}
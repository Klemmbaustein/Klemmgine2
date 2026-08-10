#pragma once
#include <ds/typeId.hpp>
#include <Engine/Objects/SceneObject.h>

namespace ds
{
	struct LanguageContext;
	struct RuntimeClass;
}

namespace engine::script
{
	extern ds::RuntimeFunction SceneObject_vTable[];
	extern ds::RuntimeFunction ObjectComponent_vTable[];

	struct EngineModuleData
	{
		ds::TypeId SceneObjectType = 0;
		ds::TypeId SceneManagerType = 0;
		ds::TypeId Vector3Type = 0;
		ds::TypeId AssetRefType = 0;
		ds::TypeId ExportAttributeType = 0;
		ds::TypeId IconAttributeType = 0;

		ds::TypeId UITextType = 0;
	};

	EngineModuleData RegisterEngineModules(ds::LanguageContext* ToContext);

	ds::RuntimeClass* CreateSceneObject(Destructible* From);
	void RegisterComponent(ds::RuntimeClass* Class);

	void UpdateWaitTasks();
}
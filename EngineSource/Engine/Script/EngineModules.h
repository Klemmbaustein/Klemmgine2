#pragma once
#include <ds/typeId.hpp>

namespace ds
{
	struct LanguageContext;
	struct RuntimeClass;
}

namespace engine::script
{
	struct EngineModuleData
	{
		ds::TypeId ScriptObjectType = 0;
		ds::TypeId Vector3Type = 0;
		ds::TypeId AssetRefType = 0;
		ds::TypeId ExportAttributeType = 0;
	};

	EngineModuleData RegisterEngineModules(ds::LanguageContext* ToContext);

	ds::RuntimeClass* CreateAssetRef();

	void UpdateWaitTasks();
}
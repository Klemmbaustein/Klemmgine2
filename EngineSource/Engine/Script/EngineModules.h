#pragma once

namespace ds
{
	struct LanguageContext;
	struct RuntimeClass;
}

namespace engine::script
{
	void RegisterEngineModules(ds::LanguageContext* ToContext);

	ds::RuntimeClass* CreateAssetRef();

	void UpdateWaitTasks();
}
#pragma once

namespace lang
{
	struct LanguageContext;
	struct RuntimeClass;
}

namespace engine::script
{
	void RegisterEngineModules(lang::LanguageContext* ToContext);

	lang::RuntimeClass* CreateAssetRef();
}
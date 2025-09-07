#pragma once

namespace lang
{
	struct LanguageContext;
}

namespace engine::script
{
	void RegisterEngineModules(lang::LanguageContext* ToContext);
}
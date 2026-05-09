#include "ScriptObject.h"
#include "EngineModules.h"
#include "ScriptSubsystem.h"
#include <ds/modules/system.async.hpp>

using namespace ds;
using ds::modules::system::async::Task;

engine::script::ScriptObject::ScriptObject(const ds::TypeInfo& Class,
	ds::InterpretContext* Interpreter)
	: Class(Class), Interpreter(Interpreter)
{
	ScriptSubsystem::Instance->BeginHotReloadEvent.Add(this, [this] {
		BeginHotReload();
	});
	ScriptSubsystem::Instance->EndHotReloadEvent.Add(this, [this] {
		EndHotReload(&ScriptSubsystem::Instance->ScriptInstructions->reflect);
	});
}

engine::script::ScriptObject::~ScriptObject()
{
	ScriptSubsystem::Instance->BeginHotReloadEvent.Remove(this);
	ScriptSubsystem::Instance->EndHotReloadEvent.Remove(this);
}

void engine::script::ScriptObject::LoadScriptData()
{
	InitializeScriptPointer();
}

void engine::script::ScriptObject::UnloadScriptData()
{
	this->ScriptData = nullptr;
}

void engine::script::ScriptObject::InitializeScriptPointer()
{
}

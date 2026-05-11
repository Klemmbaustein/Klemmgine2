#include "ScriptSceneManager.h"
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/Engine.h>

engine::script::ScriptSceneManager::ScriptSceneManager(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter)
	: ScriptObject(Class, Interpreter)
{
}

engine::script::ScriptSceneManager::~ScriptSceneManager()
{
	OnDestroyedEvent.Invoke();
	UnloadScriptData();
}

void engine::script::ScriptSceneManager::InitializeScriptPointer()
{
	InitializePointerWithValue<SceneManager*>(this);
	ScriptSubsystem::Instance->RegisterClassForObject(this, ScriptData);
}

void engine::script::ScriptSceneManager::BeginHotReload()
{
	Interpreter->destruct(ScriptData);
	UnloadScriptData();
}

void engine::script::ScriptSceneManager::EndHotReload(ds::ReflectInfo* ClassData)
{
	Class = ClassData->types[Class.hash];
	LoadScriptData();
}

void engine::script::ScriptSceneManager::Update()
{
	if (Engine::IsPlaying && ScriptData)
	{
		Interpreter->callVirtualMethodVoid(ScriptData, 1);
	}
}

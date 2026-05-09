#pragma once
#include <Engine/Objects/Scene/SceneManager.h>
#include <Engine/Script/ScriptObject.h>

namespace engine::script
{
	class ScriptSceneManager : public ScriptObject, public SceneManager
	{
	public:

		ScriptSceneManager(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter);
		~ScriptSceneManager();

		// Inherited via ScriptObject
		void InitializeScriptPointer() override;

		// Inherited via ScriptObject
		void BeginHotReload() override;
		void EndHotReload(ds::ReflectInfo* ClassData) override;
		void Update() override;
	};
}
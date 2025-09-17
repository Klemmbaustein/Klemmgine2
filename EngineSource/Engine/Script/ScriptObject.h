#pragma once
#include <Engine/Objects/SceneObject.h>
#include <class.hpp>
#include <reflection.hpp>

namespace engine::script
{
	class ScriptObject : public SceneObject
	{
	public:

		ScriptObject(const lang::TypeInfo& Class, lang::InterpretContext* Interpreter);

		lang::TypeInfo Class;
		lang::RuntimeClass* ScriptData = nullptr;
		lang::InterpretContext* Interpreter = nullptr;

		virtual void Begin();
		virtual void OnDestroyed();
		virtual void Update();

		void LoadScriptData();
		void UnloadScriptData();
	};

	struct ScriptObjectData
	{
		ScriptObject* Parent = nullptr;
		Vector3 Position = 0;
	};
}
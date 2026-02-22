#pragma once
#include <Engine/Objects/SceneObject.h>
#include <ds/class.hpp>
#include <ds/reflection.hpp>

namespace engine::script
{
	// Object running script logic
	class ScriptObject : public SceneObject
	{
	public:

		ScriptObject(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter);

		ds::TypeInfo Class;
		ds::RuntimeClass* ScriptData = nullptr;
		ds::InterpretContext* Interpreter = nullptr;

		virtual void Begin();
		virtual void OnDestroyed();
		virtual void Update();

		void LoadScriptData();
		void UnloadScriptData();
	private:
		void InitializePropertyFlags(ObjPropertyBase* p, const string& FlagsString);

		void InitializeScriptPointer();
	};
}
#pragma once
#include <Engine/Objects/SceneObject.h>
#include <ds/class.hpp>
#include <ds/reflection.hpp>

namespace engine::script
{
	/// Object running script logic
	class ScriptObject : public SceneObject
	{
	public:

		ScriptObject(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter);

		/// Type information on the class this object represents
		ds::TypeInfo Class;
		/// The class object this object represents. Has the type of `this->Class`
		ds::RuntimeClass* ScriptData = nullptr;
		/// The current interpret context to run this object's methods on.
		ds::InterpretContext* Interpreter = nullptr;

		virtual void Begin();
		virtual void OnDestroyed();
		virtual void Update();

		/**
		 * @brief
		 * Reload the Script object, creating a new instance of the type `this->Class`
		 */
		void LoadScriptData();
	private:
		void UnloadScriptData();
		void InitializePropertyFlags(ObjPropertyBase* p, const string& FlagsString);

		void InitializeScriptPointer();
	};
}
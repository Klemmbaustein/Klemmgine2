#pragma once
#include <ds/class.hpp>
#include <ds/reflection.hpp>
#include <ds/interpreter.hpp>
#include <Engine/MainThread.h>

namespace engine::script
{
	class ScriptObject
	{
	public:
		ScriptObject(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter);
		virtual ~ScriptObject();

		/// Type information on the class this object represents
		ds::TypeInfo Class;
		/// The class object this object represents. Has the type of `this->Class`
		ds::RuntimeClass* ScriptData = nullptr;
		/// The current interpret context to run this object's methods on.
		ds::InterpretContext* Interpreter = nullptr;

		/**
		 * @brief
		 * Reload the Script object, creating a new instance of the type `this->Class`
		 */
		virtual void LoadScriptData();
		void UnloadScriptData();
		virtual void InitializeScriptPointer() = 0;
		virtual void BeginHotReload() = 0;
		virtual void EndHotReload(ds::ReflectInfo* ClassData) = 0;

		template<typename T>
		void InitializePointerWithValue(T Value)
		{
			// The constructor (and for that reason also this function) may be called from another thread
			// when loading a scene asynchronously. In that case, create a new context because using the
			// main context on another thread (especially when stuff on the main thread might be using that
			// context at the same time) is a very bad idea!
			if (!thread::IsMainThread)
			{
				auto copy = Interpreter->createCopy();
				this->ScriptData = Class.create(copy);
				ds::ClassRef<T> ScriptDataRef = this->ScriptData;
				ScriptDataRef.getValue() = Value;
				delete copy;
			}
			else
			{
				this->ScriptData = Class.create(Interpreter);
				ds::ClassRef<T> ScriptDataRef = this->ScriptData;
				ScriptDataRef.getValue() = Value;
			}
		}
	};
}
#pragma once
#include <ds/class.hpp>
#include <ds/reflection.hpp>
#include <ds/interpreter.hpp>
#include <Engine/MainThread.h>

namespace engine::script
{
	/**
	 * @brief
	 * Classes inherited from this class are able to have a corresponding script object attached.
	 */
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
		/**
		 * @brief
		 * Called when a hot reload is about to start.
		 *
		 * The script object should destruct it's associated class during the hot
		 * reload, as it will not be able to do so when the hot reload is done and
		 * the old class and code information is replaced.
		 */
		virtual void BeginHotReload() = 0;

		/**
		 * @brief
		 * Called when a hot reload just ended
		 *
		 * The script object should create a new runtime object from the given reflection information.
		 *
		 * @param ClassData
		 * The reflection info of the newly compiled class
		 */
		virtual void EndHotReload(ds::ReflectInfo* ClassData) = 0;
		/**
		 * @brief
		 * Re-initialization function called after EndHotReload.
		 *
		 * This function should be used to initialize script objects after everything has been reloaded,
		 * such as calling onBegin() functions
		 */
		virtual void ReInitializeAfterHotReload() = 0;

		template<typename T>
		void InitializePointerWithValue(T Value)
		{
			// The constructor (and for that reason also this function) may be called from another thread
			// when loading a scene asynchronously. In that case, create a new context because using the
			// main context on another thread (especially when stuff on the main thread might be using that
			// context at the same time) is a very bad idea!
			// TODO: Possibly keep a refcounted context
			if (!thread::IsMainThread)
			{
				auto copy = Interpreter->createCopy();
				ScriptData = Class.create(copy);
				if (ScriptData)
				{
					ds::ClassRef<T> ScriptDataRef = ScriptData;
					ScriptDataRef.getValue() = Value;
					delete copy;
				}
			}
			else
			{
				ScriptData = Class.create(Interpreter);
				if (ScriptData)
				{
					ds::ClassRef<T> ScriptDataRef = ScriptData;
					ScriptDataRef.getValue() = Value;
				}
			}
		}
	};
}
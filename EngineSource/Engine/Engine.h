#pragma once
#include <Core/Types.h>
#include "Subsystem/Subsystem.h"
#include <typeinfo>

namespace engine
{

	/**
	* @brief
	* Class representing an instance of the engine.
	* 
	* Example usage:
	* 
	* ```cpp
	* Engine* Instance = Engine::Init();
	* 
	* Instance->Run();
	* ```
	*/
	class Engine
	{
		Engine();

	protected:
		std::vector<subsystem::Subsystem*> LoadedSystems;

	public:

		bool ShouldQuit = false;

		/**
		* @brief
		* Initializes the engine.
		* 
		* Creates a new engine instance if none exists, otherwise it returns the current engine instance.
		*/
		static Engine* Init();

		/**
		* @brief
		* Runs the engine instance, returns when Engine::ShouldQuit is set to true.
		*/
		void Run();

		/**
		* @brief
		* Adds a new subsystem to the engine.
		* 
		* @see subsystem::Subsystem
		*/
		void LoadSubsystem(subsystem::Subsystem* NewSubsystem);

		static Engine* Instance;

		template<typename T>
		static T* GetSubsystem()
		{
			for (subsystem::Subsystem* i : Instance->LoadedSystems)
			{
				if (typeid(*i) == typeid(T))
				{
					return dynamic_cast<T*>(i);
				}
			}
			return nullptr;
		}

		friend class engine::subsystem::Subsystem;

	private:
		void InitSystems();
		static void ErrorCallback(string Error, string StackTrace);
	};
}
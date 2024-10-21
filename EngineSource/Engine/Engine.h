#pragma once
#include "Types.h"
#include "Subsystem/Subsystem.h"
#include <kui/Window.h>
#include <typeinfo>
#include "Scene.h"

namespace engine
{
	class Engine
	{
		Engine();

	protected:
		std::vector<subsystem::Subsystem*> LoadedSystems;

	public:

		bool ShouldQuit = false;

		static Engine* Init();

		void Run();

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
	};
}
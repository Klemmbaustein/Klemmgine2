#pragma once
#include "Types.h"
#include "Subsystem/ISubsystem.h"
#include <typeinfo>

namespace engine
{
	class Engine
	{
		Engine();

	protected:
		std::vector<subsystem::ISubsystem*> LoadedSystems;

	public:

		bool ShouldQuit = false;

		static Engine* Init();

		void Run();

		void LoadSubsystem(subsystem::ISubsystem* NewSubsystem);

		static Engine* Instance;

		template<typename T>
		static T* GetSubsystem()
		{
			for (subsystem::ISubsystem* i : Instance->LoadedSystems)
			{
				if (typeid(*i) == typeid(T))
				{
					return dynamic_cast<T*>(i);
				}
			}
			return nullptr;
		}

		friend class engine::subsystem::ISubsystem;
	};
}
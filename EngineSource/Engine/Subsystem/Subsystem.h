#pragma once
#include "Engine/Types.h"
#include <typeinfo>
#include <Engine/Log.h>

namespace engine::subsystem
{
	class Subsystem
	{
		const char* Name = "";
		static std::vector<Subsystem*> ToUnload;
	protected:
		engine::Log::LogColor SubsystemColor = Log::LogColor::Blue;

	public:

		void Unload();
		static void UpdateUnloading();
		Subsystem(const char* Name, engine::Log::LogColor Color);
		virtual ~Subsystem();

		virtual void Update();
		virtual void RenderUpdate();

		enum class LogType
		{
			Note,
			Info,
			Warning,
			Error,
			Critical
		};

		void Print(string Message, LogType Severity = LogType::Info);

		void SubsystemDependsOn(const std::type_info& Type, string Name);
	};
}

#define THIS_SUBSYSTEM_DEPENDS_ON(SubsystemName) this->SubsystemDependsOn(typeid(SubsystemName), # SubsystemName)
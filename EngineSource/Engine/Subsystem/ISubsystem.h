#pragma once
#include "Engine/Types.h"
#include <typeinfo>

namespace engine::subsystem
{
	class ISubsystem
	{
		const char* Name = "";

	public:
		ISubsystem(const char* Name);
		virtual ~ISubsystem();

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
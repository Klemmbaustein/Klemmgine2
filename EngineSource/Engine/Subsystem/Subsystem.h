#pragma once
#include "Engine/Types.h"
#include <typeinfo>

namespace engine::subsystem
{
	class Subsystem
	{
		const char* Name = "";

	public:
		Subsystem(const char* Name);
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

		void SubsystemDependsOn(const type_info& Type, string Name);
	};
}

#define THIS_SUBSYSTEM_DEPENDS_ON(SubsystemName) this->SubsystemDependsOn(typeid(SubsystemName), # SubsystemName)
#pragma once
#include "Core/Types.h"
#include <typeinfo>
#include <Core/Log.h>

namespace engine::subsystem
{
	/**
	* @brief
	* A subsystem of the engine.
	* 
	* Each subsystem can be loaded and unloaded, and might depend on other subsystems.
	* 
	* @see Engine
	*/
	class Subsystem
	{
		const char* Name = "";
		static std::vector<Subsystem*> ToUnload;
	protected:
		engine::Log::LogColor SubsystemColor = Log::LogColor::Blue;

	public:

		/**
		* @brief
		* Unloads this subsystem.
		*/
		void Unload();
		static void UpdateUnloading();

		/**
		* @brief
		* Creates a new subsystem.
		* 
		* @param Name
		* The name of this subsystem. All log messages of this subsystem will have this string added as a prefix.
		* 
		* @param Color
		* The color of this subsystem. This is the color of the prefix of the log messages of this subsystem.
		*/
		Subsystem(const char* Name, engine::Log::LogColor Color);
		virtual ~Subsystem();

		virtual void Update();
		virtual void RenderUpdate();

		enum class LogType
		{
			/// A note message. Will be ignored unless -verbose is on.
			Note,
			/// An info message. Same as Log::Info()
			Info,
			/// A warning message. Same as Log::Warn()
			Warning,
			/// An error message. Same as Log::Warn()
			Error,
			/// A critical error message. Has the prefix [Critical]: and a red color.
			Critical
		};

		std::vector<Log::LogPrefix> GetLogPrefixes();

		/**
		* @brief
		* Prints a message with the given type into the log, and adds a prefix of the subsystem name.
		*/
		void Print(string Message, LogType Severity = LogType::Info);

		void SubsystemDependsOn(const std::type_info& Type, string Name);
	};
}

#define THIS_SUBSYSTEM_DEPENDS_ON(SubsystemName) this->SubsystemDependsOn(typeid(SubsystemName), # SubsystemName)
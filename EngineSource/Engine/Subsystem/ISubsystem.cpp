#include "ISubsystem.h"
#include <iostream>
#include <array>
#include <Engine/Engine.h>
using namespace engine::subsystem;

ISubsystem::ISubsystem(const char* Name, engine::Log::LogColor Color)
{
	Engine::Instance->LoadedSystems.push_back(this);
	this->Name = Name;
	this->SubsystemColor = Color;
	Print("Creating subsystem: " + string(Name), LogType::Note);
}

ISubsystem::~ISubsystem()
{
	Print("Destroying subsystem: " + string(Name), LogType::Note);
}

void ISubsystem::Update()
{
}

void ISubsystem::RenderUpdate()
{
}

void ISubsystem::Print(string Message, LogType Severity)
{
	static std::array<const char*, 5> SeverityStrings =
	{
		"Note",
		"Info",
		"Warn",
		"Error",
		"Critical"
	};
	static std::array<Log::LogColor, 5> SeverityColors =
	{
		Log::LogColor::Gray,
		Log::LogColor::White,
		Log::LogColor::Yellow,
		Log::LogColor::Red,
		Log::LogColor::Red
	};

	size_t SeverityIndex = size_t(Severity);

	std::string DisplayedName = this->Name;
	DisplayedName.resize(7, ' ');

	Log::PrintMsg(Message, SeverityColors[SeverityIndex],
		std::vector{
			Log::LogPrefix{ SeverityStrings[SeverityIndex], SeverityColors[SeverityIndex] },
			Log::LogPrefix{ DisplayedName, SubsystemColor },
		});
}

void ISubsystem::SubsystemDependsOn(const std::type_info& Type, string Name)
{
	for (ISubsystem* i : Engine::Instance->LoadedSystems)
	{
		if (Type == typeid(*i))
		{
			return;
		}
	}
	Print("The subsystem '" + string(this->Name) + "' depends on an unloaded subsystem: '" + Name + "'", LogType::Critical);
	abort();
}

#include "ISubsystem.h"
#include <iostream>
#include <array>
#include <Engine/Engine.h>
using namespace engine::subsystem;

ISubsystem::ISubsystem(const char* Name)
{
	Engine::Instance->LoadedSystems.push_back(this);
	this->Name = Name;
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

	std::string DisplayedName = this->Name;
	DisplayedName.resize(6, ' ');

	std::cout << "[" << SeverityStrings[size_t(Severity)] << "]: [" << DisplayedName << "]: "  << Message << std::endl;
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

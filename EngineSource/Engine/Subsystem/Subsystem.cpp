#include "Subsystem.h"
#include <iostream>
#include <array>
#include <Engine/Engine.h>
using namespace engine::subsystem;

std::vector<Subsystem*> Subsystem::ToUnload;

void engine::subsystem::Subsystem::Unload()
{
	ToUnload.push_back(this);
}

void engine::subsystem::Subsystem::UpdateUnloading()
{
	for (Subsystem* sys : ToUnload)
	{
		for (auto i = Engine::Instance->LoadedSystems.begin();
			i < Engine::Instance->LoadedSystems.end();
			i++)
		{
			if (*i == sys)
			{
				Engine::Instance->LoadedSystems.erase(i);
				break;
			}
		}
		delete sys;
	}
	ToUnload.clear();
}

Subsystem::Subsystem(const char* Name, engine::Log::LogColor Color)
{
	Engine::Instance->LoadedSystems.push_back(this);
	this->Name = Name;
	this->SubsystemColor = Color;
	Print("Creating subsystem: " + string(Name), LogType::Note);
}

Subsystem::~Subsystem()
{
	Print("Destroying subsystem: " + string(Name), LogType::Note);
}

void Subsystem::Update()
{
}

void Subsystem::RenderUpdate()
{
}

void Subsystem::Print(string Message, LogType Severity)
{
	if (Severity == LogType::Note)
		return;

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

void Subsystem::SubsystemDependsOn(const std::type_info& Type, string Name)
{
	for (Subsystem* i : Engine::Instance->LoadedSystems)
	{
		if (Type == typeid(*i))
		{
			return;
		}
	}
	Print("The subsystem '" + string(this->Name) + "' depends on an unloaded subsystem: '" + Name + "'", LogType::Critical);
	abort();
}

#include "SettingsCategory.h"
#include <Engine/MainThread.h>

using namespace engine;
using namespace engine::editor;

engine::editor::SettingsCategory::SettingsCategory(string Name)
	: Name(Name)
{
}

SerializedValue engine::editor::SettingsCategory::Serialize()
{
	return this->SettingsObject;
}

void engine::editor::SettingsCategory::DeSerialize(SerializedValue* From)
{
	this->SettingsObject = *From;

	for (auto& i : this->SettingsObject.GetObject())
	{
		SetSetting(i.Name, i.Value);
	}
	Initialized = true;
}

SerializedValue engine::editor::SettingsCategory::GetSetting(string Name, SerializedValue Default)
{
	if (SettingsObject.Contains(Name))
	{
		return SettingsObject.At(Name);
	}
	return Default;
}

void engine::editor::SettingsCategory::SetSetting(string Name, SerializedValue NewValue)
{
	if (SettingsObject.Contains(Name))
	{
		SettingsObject.At(Name) = NewValue;
	}
	else
	{
		SettingsObject.GetObject().push_back(SerializedData(Name, NewValue));
	}

	if (!Initialized)
	{
		return;
	}

	for (auto& [name, event] : PrivateListeners)
	{
		if (name == Name)
		{
			event(NewValue);
		}
	}

	for (auto& i : Listeners)
	{
		for (auto& [name, event] : i.second)
		{
			if (name == Name)
			{
				event.second->Run(std::bind(event.first, NewValue));
			}
		}
	}
}

void engine::editor::SettingsCategory::UpdateSetting(string Name)
{
	if (!SettingsObject.Contains(Name))
	{
		return;
	}

	auto& Value = SettingsObject.At(Name);

	for (auto& [name, event] : PrivateListeners)
	{
		if (name == Name)
		{
			event(Value);
		}
	}

	for (auto& i : Listeners)
	{
		for (auto& [name, event] : i.second)
		{
			if (name == Name)
			{
				event.second->Run(std::bind(event.first, Value));
			}
		}
	}
}

void engine::editor::SettingsCategory::ListenToSetting(void* Listener, string Name, SettingsListener OnChanged)
{
	this->Listeners[Listener].insert({ Name, { OnChanged, thread::MainThreadQueue } });
}

void engine::editor::SettingsCategory::ListenToSetting(void* Listener, string Name, SettingsListener OnChanged,
	thread::ThreadMessagesRef Queue)
{
	this->Listeners[Listener].insert({ Name, { OnChanged, Queue } });
}

void engine::editor::SettingsCategory::RemoveListener(void* Listener)
{
	this->Listeners.erase(Listener);
}

void engine::editor::SettingsCategory::AddPrivateListener(string Name, SettingsListener OnChanged)
{
	this->PrivateListeners.insert({Name, OnChanged});
}

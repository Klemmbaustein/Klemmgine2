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

	for (auto& i : Listeners)
	{
		for (auto& evt : i.second)
		{
			if (evt.first == Name)
			{
				if (!thread::IsMainThread)
				{
					thread::ExecuteOnMainThread(std::bind(evt.second, NewValue));
				}
				else
				{
					evt.second(NewValue);
				}
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

	for (auto& i : Listeners)
	{
		for (auto& evt : i.second)
		{
			if (evt.first == Name)
			{
				if (!thread::IsMainThread)
				{
					thread::ExecuteOnMainThread(std::bind(evt.second, Value));
				}
				else
				{
					evt.second(Value);
				}
			}
		}
	}
}

void engine::editor::SettingsCategory::ListenToSetting(void* Listener, string Name, SettingsListener OnChanged)
{
	this->Listeners[Listener].insert({ Name, OnChanged });
}

void engine::editor::SettingsCategory::RemoveListener(void* Listener)
{
	this->Listeners.erase(Listener);
}

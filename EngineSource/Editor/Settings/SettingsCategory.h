#pragma once
#include <Core/File/SerializedData.h>
#include <functional>
#include <map>

namespace engine::editor
{
	class SettingsCategory : ISerializable
	{
	public:

		using SettingsListener = std::function<void(SerializedValue)>;

		SettingsCategory(string Name);
		virtual ~SettingsCategory() = default;

		string Name;

		// Inherited via ISerializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		SerializedValue GetSetting(string Name, SerializedValue Default);
		void SetSetting(string Name, SerializedValue NewValue);

		void ListenToSetting(void* Listener, string Name, SettingsListener OnChanged);
		void RemoveListener(void* Listener);

	private:
		std::map<void*, std::map<string, SettingsListener>> Listeners;

		SerializedValue SettingsObject = std::vector<SerializedData>();
	};
}
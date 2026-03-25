#pragma once
#include <Core/File/SerializedData.h>
#include <Core/ThreadMessages.h>
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
		void UpdateSetting(string Name);

		void ListenToSetting(void* Listener, string Name, SettingsListener OnChanged);
		void ListenToSetting(void* Listener, string Name, SettingsListener OnChanged,
			thread::ThreadMessagesRef Queue);
		void RemoveListener(void* Listener);

	protected:
		void AddPrivateListener(string Name, SettingsListener OnChanged);

	private:
		std::map<string, SettingsListener> PrivateListeners;
		std::map<void*, std::map<string, std::pair<SettingsListener, thread::ThreadMessagesRef>>> Listeners;

		SerializedValue SettingsObject = std::vector<SerializedData>();

		bool Initialized = false;
	};
}
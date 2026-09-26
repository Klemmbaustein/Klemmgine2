#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>
#include "InterfaceSettings.h"
#include "ScriptSettings.h"
#include "ConsoleSettings.h"
#include "GraphicsSettings.h"
#include "EditorSettingsCategory.h"

namespace engine::editor
{
	class Settings : ISerializable
	{
	public:
		Settings();
		~Settings();

		void Save();

		void Reload();

		static Settings* GetInstance();
		static void CloseInstance();

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		void AddCategory(SettingsCategory* NewCategory);

		EditorSettings Editor;
		InterfaceSettings Interface;
		ScriptSettings Script;
		ConsoleSettings Console;
		GraphicsSettings Graphics;

	private:

		bool Loaded = false;

		string GetSettingsPath();

		std::vector<SettingsCategory*> Categories;

		static inline Settings* Instance = nullptr;
	};
}
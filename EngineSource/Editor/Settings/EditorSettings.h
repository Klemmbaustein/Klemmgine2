#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>
#include "InterfaceSettings.h"
#include "ScriptSettings.h"
#include "ConsoleSettings.h"
#include "GraphicsSettings.h"

namespace engine::editor
{
	struct Settings : ISerializable
	{
		Settings();
		~Settings();

		void Save();

		static Settings* GetInstance();

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		void AddCategory(SettingsCategory* NewCategory);

		InterfaceSettings Interface;
		ScriptSettings Script;
		ConsoleSettings Console;
		GraphicsSettings Graphics;

	private:

		string GetSettingsPath();

		std::vector<SettingsCategory*> Categories;

		static inline Settings* Instance = nullptr;
	};
}
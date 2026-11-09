#pragma once
#include "IDialogWindow.h"
#include <Engine/File/AssetRef.h>

namespace engine::editor
{
	class ProjectSettingsWindow : public DialogWindow
	{
	public:
		ProjectSettingsWindow();

		// Inherited via DialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;

		string Name = "Untitled";
		AssetRef StartupScene = AssetRef::EmptyAsset("kts");
		bool UseScriptJIT = false;
	};
}
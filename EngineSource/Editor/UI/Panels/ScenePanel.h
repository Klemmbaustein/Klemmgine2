#pragma once
#include "EditorPanel.h"
#include <Editor/UI/Elements/PropertyMenu.h>
#include <Engine/Scene.h>

namespace engine::editor
{
	class ScenePanel : public EditorPanel
	{
	public:

		ScenePanel();

		void Update() override;
		void OnResized() override;

		void LoadPropertiesFrom(Scene* Target);

	private:
		Scene* CurrentScene = nullptr;
		PropertyMenu* Properties = nullptr;
	};
}
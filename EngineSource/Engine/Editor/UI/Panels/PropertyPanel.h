#pragma once
#include "EditorPanel.h"
#include <Engine/Objects/SceneObject.h>

namespace engine::editor
{
	class PropertyPanel : public EditorPanel
	{
	public:
		PropertyPanel();

		void Update() override;
		void OnResized() override;

		void LoadPropertiesFrom(SceneObject* Object);

		SceneObject* SelectedObj = nullptr;
	};
}
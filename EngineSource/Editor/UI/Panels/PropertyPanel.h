#ifdef EDITOR
#pragma once
#include "EditorPanel.h"
#include <Engine/Objects/SceneObject.h>
#include <Editor/UI/Elements/PropertyMenu.h>

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

	private:
		PropertyMenu* Properties = nullptr;
		Transform OldObjectTransform;
	};
}
#endif
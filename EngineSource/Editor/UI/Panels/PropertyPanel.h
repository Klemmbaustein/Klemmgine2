#pragma once
#include "EditorPanel.h"
#include <Engine/Objects/SceneObject.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <kui/Timer.h>

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
		kui::Timer UpdateTimer;
	};
}

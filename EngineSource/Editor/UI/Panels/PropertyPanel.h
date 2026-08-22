#pragma once
#include "EditorPanel.h"
#include <Engine/Objects/SceneObject.h>
#include <Editor/UI/Elements/PropertyMenu.h>
#include <kui/Timer.h>

namespace engine::editor
{
	class Viewport;

	class PropertyPanel : public EditorPanel
	{
	public:
		PropertyPanel();
		~PropertyPanel();

		void Update() override;
		void OnResized() override;

		void LoadPropertiesFrom(SceneObject* Object);

		void OnThemeChanged() override;

		Viewport* CurrentView = nullptr;

	private:

		void AddEntry(ObjPropertyBase* Entry, SceneObject* Object);

		SceneObject* NewSelection = nullptr;
		PropertyMenu* Properties = nullptr;
		Transform OldObjectTransform;
		kui::Timer UpdateTimer;

		bool HasSelection = false;
	};
}

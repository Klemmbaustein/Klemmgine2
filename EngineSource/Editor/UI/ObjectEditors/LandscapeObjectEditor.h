#pragma once
#include "SceneObjectEditor.h"
#include <kui/UI/UIButton.h>
#include <Engine/Objects/LandscapeObject.h>

namespace engine::editor
{
	class LandscapeObjectEditor : public SceneObjectEditor
	{
	public:

		// Inherited via SceneObjectEditor
		kui::UIBox* ShowContextUI(Viewport* ToView, SceneObject* Object) override;
		bool HandleMouseClick(Viewport* View, bool Held) override;

		LandscapeObjectEditor();

		enum class LandscapeEditMode
		{
			Select,
			Shape,
			Flatten,
		};

		LandscapeEditMode Mode = LandscapeEditMode::Select;
		std::vector<kui::UIButton*> ModeButtons;
		LandscapeObject* Edited = nullptr;

		float BrushSize = 5.0f;
		float Intensity = 1.0f;
	};
}
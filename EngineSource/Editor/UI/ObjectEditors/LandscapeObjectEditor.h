#pragma once
#include "SceneObjectEditor.h"
#include <kui/UI/UIButton.h>

namespace engine::editor
{
	class LandscapeObjectEditor : public SceneObjectEditor
	{
	public:

		// Inherited via SceneObjectEditor
		kui::UIBox* ShowContextUI(Viewport* ToView, SceneObject* Object) override;

		LandscapeObjectEditor();

		enum class LandscapeEditMode
		{
			Select,
			Shape,
			Flatten,
		};

		LandscapeEditMode Mode = LandscapeEditMode::Select;
		std::vector<kui::UIButton*> ModeButtons;
	};
}
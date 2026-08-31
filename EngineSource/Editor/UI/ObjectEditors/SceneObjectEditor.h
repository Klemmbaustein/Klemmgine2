#pragma once
#include <kui/UI/UIBox.h>
#include <Engine/Objects/SceneObject.h>

namespace engine::editor
{
	class Viewport;

	class SceneObjectEditor
	{
	public:

		virtual ~SceneObjectEditor() = default;

		virtual kui::UIBox* ShowContextUI(Viewport* ToView, SceneObject* Object) = 0;

		virtual bool HandleMouseClick(Viewport* View, bool Held) = 0;

		ObjectTypeID Type = 0;
	};
}

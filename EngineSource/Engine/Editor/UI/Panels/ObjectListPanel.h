#ifdef EDITOR
#pragma once
#include "EditorPanel.h"
#include <Engine/Objects/SceneObject.h>
#include "ObjectListPanel.kui.hpp"

namespace engine::editor
{
	class ObjectListPanel : public EditorPanel
	{
	public:
		ObjectListPanel();

		ObjectListHeader* Heading = nullptr;
		virtual void Update() override;

		void DisplayList();

	private:

		struct ListObject
		{
			string Name;
			SceneObject* From = nullptr;
			bool Selected = false;
			std::vector<ListObject> Children;
		};

		size_t LastLength = SIZE_MAX;

		void AddListObjects(const std::vector<ListObject>& Objects, size_t Depth);
	};
}
#endif
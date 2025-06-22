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

		std::set<std::string> Collapsed;

		struct ListObject
		{
			string Name;
			SceneObject* From = nullptr;
			bool Selected = false;
			std::map<string, ListObject> Children;
		};

		size_t LastLength = SIZE_MAX;
		size_t LastSelectedLength = 0;
		SceneObject* LastSelectedObj = nullptr;
		string Filter;

		void AddListObjects(const std::map<string, ListObject>& Objects, size_t Depth, bool& LastWasSelected);
	};
}
#endif
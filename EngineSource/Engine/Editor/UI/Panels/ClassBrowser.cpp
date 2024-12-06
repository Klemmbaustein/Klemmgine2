#include "ClassBrowser.h"
#include <Engine/Objects/Reflection/ObjectReflection.h>

engine::editor::ClassBrowser::ClassBrowser()
	: ItemBrowser("Classes", "class_browser")
{
}

void engine::editor::ClassBrowser::OnBackgroundRightClick(kui::Vec2f At)
{
}

std::vector<engine::editor::ItemBrowser::Item> engine::editor::ClassBrowser::GetItems()
{
	std::vector<Item> Out;

	for (auto& i : Reflection::ObjectTypes)
	{
		Out.push_back(Item{
			.Name = i.second.Name,
			});
	}

	return Out;
}

void engine::editor::ClassBrowser::Back()
{
}

engine::string engine::editor::ClassBrowser::GetPathDisplayName()
{
	return "Classes::" + str::ReplaceChar(Path, '/', '.');
}

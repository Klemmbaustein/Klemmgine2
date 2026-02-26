#include "ClassBrowser.h"
#include <Editor/UI/EditorUI.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <Core/File/FileUtil.h>

engine::editor::ClassBrowser::ClassBrowser()
	: ItemBrowser("Classes", "ClassBrowser")
{
}

void engine::editor::ClassBrowser::OnBackgroundRightClick(kui::Vec2f At)
{
}

std::vector<engine::editor::ItemBrowser::Item> engine::editor::ClassBrowser::GetItems(string Path)
{
	if (Path.size())
	{
		Path.pop_back();
	}

	std::vector<Item> Out;

	std::map<string, std::set<string>> Paths;

	auto [FolderIcon, FolderColor] = EditorUI::GetExtIconAndColor("dir/");

	for (auto& [_, Object] : Reflection::ObjectTypes)
	{
		string Name = file::FileName(Object.Path);
		string Directory = file::FilePath(Object.Path);
		if (!Name.empty())
		{
			Paths[Directory].insert(Name);
		}
	}

	for (auto& i : Paths[Path])
	{
		Out.push_back(Item{
			.Name = i,
			.IsDirectory = true,
			.Color = FolderColor,
			.OnClick = [this, i] {
				this->Path = i + "/";
				UpdateItems();
			},
			.Image = FolderIcon,
			});
	}

	for (auto& i : Reflection::ObjectTypes)
	{
		if (i.second.Path == Path)
		{
			Out.push_back(Item{
				.Name = i.second.Name,
				.Color = 0.5,
				.Image = EditorUI::Instance->ObjectIcons.GetObjectIcon(i.second.TypeID),
				.Type = i.second.TypeID,
				});
		}
	}

	return Out;
}

void engine::editor::ClassBrowser::Back()
{
	size_t LastSlash = Path.substr(0, Path.size() - 1).find_last_of("/");
	if (LastSlash == std::string::npos)
		Path.clear();
	Path = Path.substr(0, LastSlash);
	ItemsScrollBox->GetScrollObject()->Scrolled = 0;
	UpdateItems();
}

engine::string engine::editor::ClassBrowser::GetPathDisplayName()
{
	return "Classes/" + Path;
}

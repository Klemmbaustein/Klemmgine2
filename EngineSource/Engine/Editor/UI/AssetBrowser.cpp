#ifdef EDITOR
#include "AssetBrowser.h"
#include <filesystem>
#include "DropdownMenu.h"
#include <Engine/Input.h>
#include <kui/Window.h>

static std::map<engine::string, kui::Vec3f> FileNameColors =
{
	{"kscn", kui::Vec3f(0)},
	{"", kui::Vec3f(0.5f)},
	{"<dir>", kui::Vec3f(0.8f, 0.5f, 0) },
};
static std::map<engine::string, engine::string> FileNameIcons =
{
	{"kscn", ""},
	{"", "Engine/Editor/Assets/Document.png"},
	{"<dir>", "Engine/Editor/Assets/Folder.png"},
};

engine::editor::AssetBrowser::AssetBrowser()
	: ItemBrowser("Assets", "asset_browser")
{
}
std::vector<engine::editor::AssetBrowser::Item> engine::editor::AssetBrowser::GetItems()
{
	std::vector<Item> Out;

	if (!std::filesystem::exists("Assets/"))
	{
		std::filesystem::create_directories("Assets/");
	}

	for (auto& i : std::filesystem::directory_iterator("Assets/" + Path))
	{
		string Extension = i.path().has_extension() ? i.path().extension().string() : "";
		std::function<void()> OnClick = nullptr;

		if (i.is_directory())
		{
			Extension = "<dir>";
			OnClick = [this, i]()
				{
					Path.append(i.path().filename().string() + "/");
					UpdateItems();
				};
		}

		string Image = "";

		if (FileNameIcons.contains(Extension))
		{
			Image = FileNameIcons[Extension];
		}
		else
		{
			Image = FileNameIcons[""];
		}


		auto OnRightClick = []()
			{
				new DropdownMenu({
					DropdownMenu::Option{
						.Name = "Open"
					}
				}, kui::Window::GetActiveWindow()->Input.MousePosition);

			};

		Out.push_back(Item{
			.Name = i.path().filename().string(),
			.OnRightClick = OnRightClick,
			.OnClick = OnClick,
			.Image = Image,
			.Color = FileNameColors[i.is_directory() ? "<dir>" : ""],
			});
	}

	return Out;
}
void engine::editor::AssetBrowser::Back()
{
	size_t LastSlash = Path.substr(0, Path.size() - 1).find_last_of("/");
	if (LastSlash == std::string::npos)
		Path.clear();
	Path = Path.substr(0, LastSlash);
	UpdateItems();
}
void engine::editor::AssetBrowser::OnBackgroundRightClick(kui::Vec2f Position)
{
	new DropdownMenu({DropdownMenu::Option{.Name = "New..."}}, Position);
}
#endif
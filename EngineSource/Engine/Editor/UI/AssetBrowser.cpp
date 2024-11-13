#ifdef EDITOR
#include "AssetBrowser.h"
#include <filesystem>
#include "DropdownMenu.h"
#include <Engine/Input.h>
#include <Engine/Internal/Platform.h>
#include <kui/Window.h>
#include <Engine/Editor/ModelConverter.h>
#include <algorithm>
#include <iostream>
#include <thread>

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
		string FilePath = i.path().string();

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


		auto OnRightClick = [this, OnClick, FilePath]()
			{
				new DropdownMenu({
					DropdownMenu::Option{
						.OnClicked = OnClick,
						.Name = "Open",
					},
					DropdownMenu::Option{
						.OnClicked = [this, FilePath]()
						{
							std::filesystem::remove_all(FilePath);
							UpdateItems();
						},
						.Name = "Delete",
					}
					}, kui::Window::GetActiveWindow()->Input.MousePosition);

			};

		Out.push_back(Item{
			.Name = i.path().filename().string(),
			.IsDirectory = bool(i.is_directory()),
			.Color = FileNameColors[i.is_directory() ? "<dir>" : ""],
			.OnRightClick = OnRightClick,
			.OnClick = OnClick,
			.Image = Image,
			});
	}

	std::sort(Out.begin(), Out.end(), [](const Item& a, const Item& b)
		{
			if (a.IsDirectory && !b.IsDirectory)
				return true;
			if (!a.IsDirectory && b.IsDirectory)
				return false;
			return a.Name > b.Name;
		});

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

static void ImportThread(engine::string CurrentPath)
{
	using namespace engine;
	using namespace engine::editor;
	using namespace engine::internal;

	auto Contains = [](string Value, const std::vector<string>& Values) -> bool
		{
			bool Found = false;
			std::ranges::for_each(Values, [&Value, &Found](const string& Current)
				{
					if (Current == Value)
						Found = true;
				});
			return Found;
		};

	std::vector<string> ModelExtensions = { "glb", "gltf", "obj", "fbx", "ply", "blend", "raw", "stl", "3ds", "amf", "assbin", "b3d", "dfx" };

	string File = platform::OpenFileDialog({
		platform::FileDialogFilter{
			.Name = "3D Models",
			.FileTypes = ModelExtensions,
		},
		platform::FileDialogFilter{
			.Name = "Images",
			.FileTypes = {"png", "jpg", "bmp"}
		},
		platform::FileDialogFilter{
			.Name = "Sound",
			.FileTypes = {"wav"}
		},
		platform::FileDialogFilter{
			.Name = "Klemmgine 2 Assets",
			.FileTypes = {"kmdl", "kscn"}
		},
		platform::FileDialogFilter{
			.Name = "_ALL",
		},
		platform::FileDialogFilter{
			.Name = "Any",
			.FileTypes = {"*"}
		} });

	string Extension = File.substr(File.find_last_of(".") + 1);

	if (Contains(Extension, ModelExtensions))
		modelConverter::ConvertModel(File, CurrentPath, modelConverter::ConvertOptions{});
}

static void Import(engine::string CurrentPath)
{
	std::thread Thread = std::thread(ImportThread, CurrentPath);
	Thread.detach();
}

void engine::editor::AssetBrowser::OnBackgroundRightClick(kui::Vec2f Position)
{
	using namespace engine::internal;

	new DropdownMenu(
		{
			DropdownMenu::Option{.OnClicked = [this]() { Import("Assets/" + Path); }, .Name = "Import...", .Separator = true},
			DropdownMenu::Option{.Name = "New scene"},
			DropdownMenu::Option{.Name = "New material", .Separator = true},
			DropdownMenu::Option{.OnClicked = [this]()
		{
			platform::Execute("explorer.exe \".\\Assets\\" + str::ReplaceChar(Path, '/', '\\') + "\"");
		}, .Name = "Open in file explorer"},
		},
		Position);
}
#endif
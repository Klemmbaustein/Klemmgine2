#ifdef EDITOR
#include "AssetBrowser.h"
#include "Viewport.h"
#include "Assets/ModelEditor.h"
#include "Assets/MaterialEditor.h"
#include <Editor/ModelConverter.h>
#include <Editor/UI/DropdownMenu.h>
#include <Editor/UI/Panels/Assets/TextEditorPanel.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Windows/ProgressBar.h>
#include <Editor/UI/Windows/RenameWindow.h>
#include <Core/File/FileUtil.h>
#include <Engine/Internal/PlatformGraphics.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Core/Platform/Platform.h>
#include <Engine/File/Resource.h>
#include <Engine/MainThread.h>
#include <kui/Window.h>
#include <filesystem>
#include <thread>
#include <algorithm>

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

	for (auto& i : std::filesystem::directory_iterator(GetPathDisplayName()))
	{
		string FilePath = i.path().string();

		string Extension = i.path().has_extension() ? i.path().extension().string().substr(1) : "";
		std::function<void()> OnClick = nullptr;

		if (i.is_directory())
		{
			Extension = "dir/";
			OnClick = [this, i]()
			{
				Path.append(i.path().filename().string() + "/");
				ItemsScrollBox->GetScrollObject()->Scrolled = 0;
				UpdateItems();
			};
		}
		else if (Extension == "kts")
		{
			OnClick = [this, FilePath]()
			{
				if (Scene::AsyncLoads)
					return;
				delete Scene::GetMain();
				subsystem::SceneSubsystem::Current->LoadSceneAsync(FilePath);
				EditorUI::SetStatusMessage("Loading Scene: " + FilePath, EditorUI::StatusType::Info);
			};
		}
		else if (Extension == "kmdl")
		{
			OnClick = [this, FilePath]()
			{
				Viewport::Current->AddChild(new ModelEditor(AssetRef::FromPath(FilePath)), Align::Tabs, true);
			};
		}
		else if (Extension == "kmt")
		{
			OnClick = [this, FilePath]()
			{
				Viewport::Current->AddChild(new MaterialEditor(AssetRef::FromPath(FilePath)), Align::Tabs, true);
			};
		}
		else if (Extension == "png")
		{
			OnClick = [this, FilePath]()
			{
#if WINDOWS
				platform::Open(str::ReplaceChar(FilePath, '/', '\\'));
#else
				platform::Open(FilePath);
#endif
			};
		}

		auto IconAndColor = EditorUI::GetExtIconAndColor(i.is_directory() ? "dir/" : Extension);

		auto OnRightClick = [this, OnClick, FilePath, Extension]() {

			std::vector<DropdownMenu::Option> Options;
			Options.push_back(DropdownMenu::Option{
				.Name = "Open",
				.Icon = EditorUI::Asset("TabDrag.png"),
				.OnClicked = OnClick,
				});

			if (Extension != "dir/")
			{
				Options.push_back(DropdownMenu::Option{
					.Name = "View raw",
					.Icon = "",
					.OnClicked = [FilePath]
					{
						Viewport::Current->AddChild(
							new TextEditorPanel(AssetRef::FromPath(FilePath)),
							Align::Tabs, true);
					},
					});
			}

			Options.push_back(DropdownMenu::Option{
				.Name = "Rename",
				.Icon = EditorUI::Asset("Rename.png"),
				.OnClicked = [this, FilePath]()
				{
					new RenameWindow(FilePath, [this, FilePath](string NewPath)
					{
						std::filesystem::rename(FilePath, NewPath);
						resource::ScanForAssets();
						UpdateItems();
					});
				},
				});

			Options.push_back(DropdownMenu::Option{
				.Name = "Delete",
				.Icon = EditorUI::Asset("X.png"),
				.OnClicked = [this, FilePath]()
				{
					std::filesystem::remove_all(FilePath);
					resource::ScanForAssets();
					UpdateItems();
				},
				});

			new DropdownMenu(Options, kui::Window::GetActiveWindow()->Input.MousePosition);
		};

		Out.push_back(Item{
			.Name = file::FileNameWithoutExt(i.path().filename().string()),
			.Path = i.path().string(),
			.IsDirectory = bool(i.is_directory()),
			.Color = IconAndColor.second,
			.OnRightClick = OnRightClick,
			.OnClick = OnClick,
			.Image = IconAndColor.first,
			});
	}

	std::sort(Out.begin(), Out.end(), [](const Item& a, const Item& b)
	{
		if (a.IsDirectory && !b.IsDirectory)
			return true;
		if (!a.IsDirectory && b.IsDirectory)
			return false;
		return str::Lower(a.Name) < str::Lower(b.Name);
	});

	SetStatusText(str::Format("%i Items", int(Out.size())));

	return Out;
}

void engine::editor::AssetBrowser::Back()
{
	size_t LastSlash = Path.substr(0, Path.size() - 1).find_last_of("/");
	if (LastSlash == std::string::npos)
		Path.clear();
	Path = Path.substr(0, LastSlash);
	ItemsScrollBox->GetScrollObject()->Scrolled = 0;
	UpdateItems();
}

static std::vector<engine::string> ModelExtensions = { "glb", "gltf", "obj", "fbx", "ply", "blend", "raw", "stl", "3ds", "amf", "assbin", "b3d", "dfx" };

static void ImportItem(engine::string File, engine::string CurrentPath)
{
	using namespace engine;
	using namespace engine::editor;

	string Extension = str::Lower(File.substr(File.find_last_of(".") + 1));

	auto Contains = [](string Value, const std::vector<string>& Values) -> bool
	{
		bool Found = false;
		for (auto& Current : Values)
		{
			if (Current == Value)
				Found = true;
		}
		return Found;
	};

	if (Contains(Extension, ModelExtensions))
	{
		EditorUI::SetStatusMessage(str::Format("Importing file: '%s'", File.c_str()), EditorUI::StatusType::Info);
		auto* Progress = new ProgressBar("Importing " + Extension + " file");
		Progress->Progress = -1;
		string Out = modelConverter::ConvertModel(File, CurrentPath, modelConverter::ConvertOptions{
			.OnLoadStatusChanged = [Progress](string NewMessage)
			{
				Progress->SetMessage("Importing model: " + NewMessage);
			}
			});
		EditorUI::SetStatusMessage(str::Format("Imported '%s' to '%s'", File.c_str(), Out.c_str()), EditorUI::StatusType::Info);
		Progress->Close();

		thread::ExecuteOnMainThread([]() {
			resource::ScanForAssets();
			EditorUI::ForEachPanel<AssetBrowser>([](AssetBrowser* Browser) {
				Browser->UpdateItems();
			});
		});
	}
	else
	{
		std::filesystem::copy(File, CurrentPath + file::FileName(CurrentPath));
		thread::ExecuteOnMainThread([]() {
			resource::ScanForAssets();
			EditorUI::ForEachPanel<AssetBrowser>([](AssetBrowser* Browser) {
				Browser->UpdateItems();
			});
		});
	}
}

static void ImportThread(engine::string CurrentPath)
{
	using namespace engine;
	using namespace engine::internal;


	std::vector Files = platform::OpenFileDialog({
		platform::FileDialogFilter{
			.Name = "3D Models",
			.FileTypes = ModelExtensions,
		},
		platform::FileDialogFilter{
			.Name = "Images",
			.FileTypes = { "png", "jpg", "bmp" }
		},
		platform::FileDialogFilter{
			.Name = "Sound",
			.FileTypes = { "wav" }
		},
		platform::FileDialogFilter{
			.Name = "Klemmgine 2 Assets",
			.FileTypes = { "kmdl", "kts", "kbs" }
		},
		platform::FileDialogFilter{
			.Name = "_ALL",
		},
		platform::FileDialogFilter{
			.Name = "Any",
			.FileTypes = { "*" }
		} });

	for (auto& i : Files)
	{
		ImportItem(i, CurrentPath);
	}
}

static void Import(engine::string CurrentPath)
{
	std::thread Thread = std::thread(ImportThread, CurrentPath);
	Thread.detach();
}

void engine::editor::AssetBrowser::OnBackgroundRightClick(kui::Vec2f Position)
{
	new DropdownMenu(
		{
		DropdownMenu::Option{
			.Name = "Import...",
			.Icon = EditorUI::Asset("Object.png"),
			.OnClicked = [this]() { Import(GetPathDisplayName()); },
			.Separator = true
		},
		DropdownMenu::Option{
			.Name = "New folder",
			.Icon = EditorUI::GetExtIconAndColor("dir/").first,
			.OnClicked = [this]() {
				std::filesystem::create_directories(GetPathDisplayName() + "/Folder");
				UpdateItems();
			}
		},
		DropdownMenu::Option{
			.Name = "New scene",
			.Icon = EditorUI::GetExtIconAndColor("kts").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Scene", "kts");
				resource::ScanForAssets();
				UpdateItems();
			}
		},
		DropdownMenu::Option{
			.Name = "New material",
			.Icon = EditorUI::GetExtIconAndColor("kmt").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Material", "kmt");
				resource::ScanForAssets();
				UpdateItems();
			},
			.Separator = true },
		DropdownMenu::Option{
			.Name = "Open in file explorer",
			.Icon = EditorUI::Asset("TabDrag.png"),
			.OnClicked = [this]()
			{
				using namespace engine::platform;
#if WINDOWS
				Execute("explorer.exe \".\\Assets\\" + str::ReplaceChar(Path, '/', '\\') + "\"");
#else
				std::thread([this]() {Execute("xdg-open \"./Assets/" + Path + "\""); }).detach();
#endif
		}, },
		},
		Position);
}

void engine::editor::AssetBrowser::OnItemsRightClick(kui::Vec2f MousePosition)
{
	new DropdownMenu({
	DropdownMenu::Option{
		.Name = "Delete",
		.OnClicked = [this]()
		{
			auto Selected = GetSelected();
			for (Item* i : Selected)
			{
				std::filesystem::remove_all(i->Path);
			}
			resource::ScanForAssets();
			UpdateItems();
		},
	} }, MousePosition);

}

engine::string engine::editor::AssetBrowser::GetPathDisplayName()
{
	return "Assets/" + Path;
}
#endif
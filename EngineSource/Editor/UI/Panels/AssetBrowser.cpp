#include "AssetBrowser.h"
#include "Viewport.h"
#include "Assets/ModelEditor.h"
#include "Assets/MaterialEditor.h"
#include "ScriptEditorPanel.h"
#include <Editor/ModelConverter.h>
#include <Editor/UI/DropdownMenu.h>
#include <Editor/UI/Panels/Assets/TextEditorPanel.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Windows/ProgressBar.h>
#include <Editor/UI/Windows/RenameWindow.h>
#include <Editor/UI/Windows/MessageWindow.h>
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

static void Import(engine::string CurrentPath);

engine::editor::AssetBrowser::AssetBrowser()
	: ItemBrowser("Assets", "asset_browser")
{
	AddShortcut(kui::Key::F2, {}, [this] {
		for (auto& i : this->Buttons)
		{
			if (i.first.Selected)
			{
				RenameFile(i.first.Path);
				return;
			}
		}
	});

	AddShortcut(kui::Key::DELETE, {}, [this] {
		for (auto& i : this->Buttons)
		{
			if (i.first.Selected)
			{
				new MessageWindow(str::Format("Really delete %s?", i.first.Path.c_str()), "Delete file", {
					IDialogWindow::Option{
						.Name = "Yes",
						.OnClicked = [this, i]() {
							EditorUI::Instance->AssetsProvider->DeleteFile(i.first.Path);
							resource::ScanForAssets();
							UpdateItems();
						},
						.IsAccept = true,
					},
					IDialogWindow::Option{
						.Name = "No",
						.IsClose = true,
					}
					});
				return;
			}
		}
	});

	AddShortcut(kui::Key::i, kui::Key::CTRL, [this] {
		Import(GetPathDisplayName());
	});

	AddShortcut(kui::Key::d, kui::Key::CTRL, [this] {
		EditorUI::CreateDirectory(GetPathDisplayName() + "Folder");
	});

	AddShortcut(kui::Key::n, kui::Key::CTRL, [this] {
		EditorUI::CreateAsset(GetPathDisplayName(), "Scene", "kts");
		resource::ScanForAssets();
	});

	AddShortcut(kui::Key::m, kui::Key::CTRL, [this] {
		EditorUI::CreateAsset(GetPathDisplayName(), "Material", "kmt");
		resource::ScanForAssets();
	});

	AddShortcut(kui::Key::t, kui::Key::CTRL, [this] {
		EditorUI::CreateAsset(GetPathDisplayName(), "Fragment", "frag");
		resource::ScanForAssets();
	});
}

std::vector<engine::editor::AssetBrowser::Item> engine::editor::AssetBrowser::GetItems(string Path)
{
	std::vector<Item> Out;

	auto Items = EditorUI::Instance->AssetsProvider->GetFiles("Assets/" + Path);

	EditorUI::Instance->AssetsProvider->OnChanged.Add(this, [this] {
		UpdateItems();
	});

	for (AssetFile& File : Items)
	{
		string FilePath = File.Path;
		string Extension = file::Extension(File.Path);
		std::function<void()> OnClick = nullptr;

		if (File.IsDirectory)
		{
			Extension = "dir/";
			OnClick = [this, File]()
			{
				this->Path.append(file::FileName(File.Path) + "/");
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
				SceneSubsystem::Current->LoadSceneAsync(FilePath);
				EditorUI::SetStatusMessage("Loading Scene: " + FilePath, EditorUI::StatusType::Info);
			};
		}
		else if (Extension == "ds" || Extension == "kui")
		{
			OnClick = [this, FilePath]()
			{
				EditorUI::ForEachPanel<ScriptEditorPanel>([FilePath](ScriptEditorPanel* p) {
					p->NavigateTo(FilePath, {});
					p->SetFocused();
				});
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

		auto IconAndColor = EditorUI::GetExtIconAndColor(Extension);

		auto OnRightClick = [this, OnClick, FilePath, Extension]() {

			std::vector<DropdownMenu::Option> Options;
			Options.push_back(DropdownMenu::Option{
				.Name = "Open",
				.Icon = EditorUI::Asset("Open.png"),
				.OnClicked = OnClick,
				});

			if (Extension != "dir/")
			{
				Options.push_back(DropdownMenu::Option{
					.Name = "View text",
					.Icon = EditorUI::Asset("Code.png"),
					.OnClicked = [FilePath]
					{
						Viewport::Current->AddChild(
							new TextEditorPanel(AssetRef::FromPath(FilePath)),
							Align::Tabs, true);
					},
					});
			}

			Options.push_back(DropdownMenu::Option{
				.Name = "Duplicate",
				.Icon = EditorUI::Asset("Copy.png"),
				.OnClicked = std::bind(&AssetBrowser::DuplicateFile, this, FilePath),
				});

			Options.push_back(DropdownMenu::Option{
				.Name = "Rename",
				.Shortcut = "F2",
				.Icon = EditorUI::Asset("Rename.png"),
				.OnClicked = std::bind(&AssetBrowser::RenameFile, this, FilePath),
				});

			Options.push_back(DropdownMenu::Option{
				.Name = "Delete",
				.Shortcut = "Del",
				.Icon = EditorUI::Asset("X.png"),
				.OnClicked = [this, FilePath]()
				{
					EditorUI::Instance->AssetsProvider->DeleteFile(FilePath);
					resource::ScanForAssets();
					UpdateItems();
				},
				});

			new DropdownMenu(Options, kui::Window::GetActiveWindow()->Input.MousePosition);
		};

		Out.push_back(Item{
			.Name = File.IsDirectory ? file::FileName(File.Path) : file::FileNameWithoutExt(File.Path),
			.Path = File.Path,
			.IsDirectory = File.IsDirectory,
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

static std::vector<engine::string> ModelExtensions = { "glb", "gltf", "obj", "fbx", "ply",
"blend", "raw", "stl", "3ds", "amf", "assbin", "b3d", "dfx" };

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
			.Shortcut = "Ctrl+I",
			.Icon = EditorUI::Asset("Plus.png"),
			.OnClicked = [this]() { Import(GetPathDisplayName()); },
			.Separator = true
		},
		DropdownMenu::Option{
			.Name = "New folder",
			.Shortcut = "Ctrl+D",
			.Icon = EditorUI::GetExtIconAndColor("dir/").first,
			.OnClicked = [this]() {
				EditorUI::CreateDirectory(GetPathDisplayName() + "Folder");
			}
		},
		DropdownMenu::Option{
			.Name = "New scene",
			.Shortcut = "Ctrl+N",
			.Icon = EditorUI::GetExtIconAndColor("kts").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Scene", "kts");
				resource::ScanForAssets();
			}
		},
		DropdownMenu::Option{
			.Name = "New material",
			.Shortcut = "Ctrl+M",
			.Icon = EditorUI::GetExtIconAndColor("kmt").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Material", "kmt");
				resource::ScanForAssets();
			},
		},
		DropdownMenu::Option{
			.Name = "New script",
			.Shortcut = "Ctrl+E",
			.Icon = EditorUI::GetExtIconAndColor("").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Script", "ds");
				resource::ScanForAssets();
			},
		},
		DropdownMenu::Option{
			.Name = "New fragment shader",
			.Shortcut = "Ctrl+T",
			.Icon = EditorUI::GetExtIconAndColor("kmt").first,
			.OnClicked = [this]() {
				EditorUI::CreateAsset(GetPathDisplayName(), "Fragment", "frag");
				resource::ScanForAssets();
			},
			.Separator = true
		},
		DropdownMenu::Option{
			.Name = "Open in file explorer",
			.Icon = EditorUI::Asset("Open.png"),
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
				EditorUI::Instance->AssetsProvider->DeleteFile(i->Path);
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

void engine::editor::AssetBrowser::DuplicateFile(string FilePath)
{
	if (std::filesystem::is_directory(FilePath))
	{
		std::filesystem::copy(FilePath, file::FilePath(FilePath) + " (Copy)");
	}
	else
	{
		std::filesystem::copy(FilePath, file::FilePath(FilePath) + "/" +
			file::FileNameWithoutExt(FilePath) + " (Copy)." + file::Extension(FilePath));
	}
	resource::ScanForAssets();
	UpdateItems();
}

void engine::editor::AssetBrowser::RenameFile(string FilePath)
{
	new RenameWindow(FilePath, [this, FilePath](string NewPath)
	{
		std::filesystem::rename(FilePath, NewPath);
		resource::ScanForAssets();
		UpdateItems();
	});
}

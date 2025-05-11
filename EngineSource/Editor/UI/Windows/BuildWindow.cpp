#include "BuildWindow.h"
#include <Editor/UI/EditorUI.h>
#include <Editor/UI/Elements/Checkbox.h>
#include <kui/UI/UIDropdown.h>
#include <kui/UI/UISpinner.h>
#include <Engine/Version.h>
#include <Core/Platform/Platform.h>
#include <DialogWindow.kui.hpp>
#include <thread>

using namespace engine::editor;
using namespace kui;

engine::editor::BuildWindow::BuildWindow()
	: IDialogWindow("Build project",
		{
			Option{.Name = "Build", .OnClicked = std::bind(&BuildWindow::StartBuild, this), .Close = false, .OnMainThread = false},
			Option{.Name = "Cancel", .Close = true},
		}
		, kui::Vec2ui(400, 500))
{
	this->Open();
}

void engine::editor::BuildWindow::Begin()
{
	IDialogWindow::Begin();
	Background->SetHorizontal(false);

	std::map<string, BuildPlatform> Platforms = {
		{"Windows", BuildPlatform::Windows},
		{"Linux", BuildPlatform::Linux},
	};

#if WINDOWS
	AddBuildHeader("Target Platforms", Background);

	AddCheckbox("Windows (x86_64)", BuildForWindows, Background);
	AddCheckbox("Windows (ARM64)", BuildForWindowsArm, Background);
	AddCheckbox("Linux (x86_64)", BuildForLinux, Background);

	AddBuildText("'Linux (x86_64)' requires WSL. The Windows platforms require a compatible Compiler.", Background);
#endif


	AddBuildHeader("Build settings", Background);

	std::vector ConfigOptions = {
		UIDropdown::Option("Debug"),
		UIDropdown::Option("Release"),
		UIDropdown::Option("Release (Size optimized)"),
		UIDropdown::Option("Release (with Debug info)"),
	};

	auto ConfigDropdown = new UIDropdown(0, 200_px,
		EditorUI::Theme.DarkBackground, EditorUI::Theme.Text,
		ConfigOptions, nullptr, DefaultFont);

	ConfigDropdown->OnClicked = [this, ConfigDropdown]() {
		switch (ConfigDropdown->SelectedIndex)
		{
		case 0:
			this->SelectedConfig = "Debug";
			break;
		case 1:
			this->SelectedConfig = "Release";
			break;
		case 2:
			this->SelectedConfig = "MinSizeRel";
			break;
		case 3:
			this->SelectedConfig = "RelWithDebInfo";
			break;
		default:
			break;
		}
		};

	ConfigDropdown->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	ConfigDropdown->SetTextSize(11_px, 3_px);
	ConfigDropdown->SelectOption(0, false);

	AddElement("Configuration", ConfigDropdown, Background);

	AddCheckbox("Compress assets", CompressAssets, Background);
	AddCheckbox("Include dev plugins", IncludeDevPlugins, Background);
	AddCheckbox("Multi threaded build", MultiThreaded, Background);

	AddSeparator(Background);

	AddBuildText(VersionInfo::Get().GetDisplayNameAndBuild(), Background);
}

void engine::editor::BuildWindow::Update()
{
	bool AllDone = !Jobs.empty();
	for (auto& i : Jobs)
	{
		i->Update();
		if (!i->IsDone)
		{
			AllDone = false;
		}
	}

	if (AllDone)
	{
		Jobs.clear();
		SetButtons({ Option{
			.Name = "Show result",
			.OnClicked = []() {
#if WINDOWS
			platform::Execute("explorer.exe \".\\build\"");
#endif
			},
			.Close = false,
			.OnMainThread = true,
		},
		Option{
			.Name = "Close",
			.Close = true,
		} });
	}
}

void BuildWindow::BuildJob::Update()
{
	std::lock_guard g{ CurrentStatusMutex };

	if (!StatusText)
	{
		return;
	}

	StatusText->SetText(this->CurrentStatus);

	if (CurrentStage != OldStage)
	{
		OldStage = CurrentStage;
		switch (CurrentStage)
		{
		case engine::editor::BuildStage::Configure:
			NextBuildStage("Configure");
			break;
		case engine::editor::BuildStage::Compile:
			NextBuildStage("Compiling");
			break;
		case engine::editor::BuildStage::CreateBuild:
			NextBuildStage("Creating build");
			break;
		default:
			NextBuildStage("");
			break;
		}
	}
}

void engine::editor::BuildWindow::BuildJob::Start()
{
	NextBuildStage("Configure");

	std::thread BuildThread = std::thread(&BuildJob::StartBuildThread, this);
	BuildThread.detach();
}

void engine::editor::BuildWindow::BuildJob::AddLine(string NewLine, BuildStage Stage)
{
	if (Stage != BuildStage::Done && str::Trim(NewLine).empty())
		return;

	std::lock_guard g{ CurrentStatusMutex };

	if (NewLine.size() > 3 && NewLine.substr(0, 3) == "-- ")
	{
		NewLine = NewLine.substr(3);
	}

	this->CurrentStage = Stage;
	this->CurrentStatus = str::Trim(NewLine);
}

void engine::editor::BuildWindow::Destroy()
{
}

void engine::editor::BuildWindow::StartBuildForPlatform(BuildPlatform Platform, kui::UIScrollBox* ParentScrollBox)
{
	auto& NewJob = Jobs.emplace_back(std::make_shared<BuildJob>());

	UIBackground* bg = new UIBackground(false, 0, EditorUI::Theme.Background);

	bg->SetBorder(1_px, EditorUI::Theme.Highlight1)
		->SetCorner(5_px)
		->SetPadding(5_px)
		->SetMinWidth(UISize::Parent(1))
		->SetMinHeight(10_px);
	ParentScrollBox->AddChild(bg);

	bg->AddChild((new UIBackground(true, 0, EditorUI::Theme.HighlightDark))
		->SetBorder(1_px, EditorUI::Theme.Highlight1)
		->SetCorner(5_px)
		->SetBorderEdges(true, false, true, true)
		->SetCorners(true, true, false, false)
		->SetMinWidth(UISize::Parent(1))
		->AddChild((new UIText(13_px, 1, "Building - " + GetPlatformDisplayName(Platform), this->DefaultFont))
			->SetPadding(5_px)));

	NewJob->JobBackground = new UIBox(false, 0);

	bg->AddChild(NewJob->JobBackground
		->SetMinWidth(UISize::Parent(1))
		->SetPadding(0, 15_px, 0, 0));

	NewJob->Options = BuildOptions{
		.Platform = Platform,
		.CompressAssets = CompressAssets,
		.IncludeDevPlugins = IncludeDevPlugins,
		.MultiThreaded = MultiThreaded,
		.OutputPath = "build/" + SelectedConfig,
		.BuildConfiguration = SelectedConfig,
	};
	NewJob->Parent = this;
	NewJob->Start();
}

void engine::editor::BuildWindow::AddBuildHeader(string Message, kui::UIBox* To)
{
	To->AddChild((new UIText(13_px, EditorUI::Theme.Text, Message, DefaultFont))
		->SetWrapEnabled(true, 370_px)
		->SetPadding(10_px, 0_px, 10_px, 10_px));
	AddSeparator(To);
}

void engine::editor::BuildWindow::AddBuildText(string Text, kui::UIBox* To)
{
	To->AddChild((new UIText(11_px, EditorUI::Theme.DarkText, Text, DefaultFont))
		->SetWrapEnabled(true, 370_px)
		->SetPadding(10_px, 0_px, 15_px, 15_px));
}

void engine::editor::BuildWindow::AddCheckbox(string Name, bool& Value, kui::UIBox* To)
{
	auto CheckBox = new UICheckbox(Value, nullptr);

	CheckBox->OnClicked = [CheckBox, &Value]() {
		Value = CheckBox->Value;
		};

	AddElement(Name, CheckBox, To);
}

void engine::editor::BuildWindow::AddElement(string Name, kui::UIBox* Element, kui::UIBox* To)
{
	To->AddChild((new UIBox(true, 0))
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetPadding(4_px, 4_px, 15_px, 15_px)
		->AddChild((new UIText(11_px, EditorUI::Theme.Text, Name, DefaultFont))
			->SetTextWidthOverride(Name.empty() ? 0 : 150_px))
		->AddChild(Element));
}

engine::string engine::editor::BuildWindow::GetPlatformDisplayName(BuildPlatform Platform)
{
	switch (Platform)
	{
	case engine::editor::BuildPlatform::Windows:
		return "Windows x86_64";
	case engine::editor::BuildPlatform::WindowsArm:
		return "Windows arm64";
	case engine::editor::BuildPlatform::Linux:
		return "Linux x86_64";
	default:
		break;
	}
	return string();
}

void BuildWindow::BuildWindow::AddSeparator(kui::UIBox* To)
{
	To->AddChild((new UIBackground(true, 0, EditorUI::Theme.BackgroundHighlight, SizeVec(UISize::Parent(1), 1_px)))
		->SetPadding(5_px, 5_px, 5_px, 5_px));
}

void engine::editor::BuildWindow::StartBuild()
{
	Background->DeleteChildren();
	SetButtons({});

	auto ScrollBox = new UIScrollBox(false, 0, true);
	ScrollBox->SetMinSize(UISize::Parent(1));
	ScrollBox->SetMaxSize(UISize::Parent(1));
	Background->AddChild(ScrollBox);

#if WINDOWS
	if (BuildForWindows)
	{
		StartBuildForPlatform(BuildPlatform::Windows, ScrollBox);
	}
	if (BuildForWindowsArm)
	{
		StartBuildForPlatform(BuildPlatform::WindowsArm, ScrollBox);
	}
#endif
	if (BuildForLinux)
	{
		StartBuildForPlatform(BuildPlatform::Linux, ScrollBox);
	}
}

void BuildWindow::BuildJob::StartBuildThread()
{
	this->Options.LogLineAdded = [this](string NewLog, BuildStage Stage) {
		this->AddLine(NewLog, Stage);
		};
	BuildCurrentProject(this->Options);
}

void BuildWindow::BuildJob::BuildDone()
{
	StatusText = nullptr;
	IsDone = true;

	Parent->AddBuildHeader("Result", JobBackground);
	Parent->AddBuildText(str::Format("Build completed.\nTook %i seconds.", int32(BuildTimer.Get())), JobBackground);
}

void BuildWindow::BuildJob::NextBuildStage(string StageName)
{
	if (ProgressBox)
	{
		delete ProgressBox;
		if (CurrentStage == BuildStage::Failed)
		{
			Parent->AddBuildText(this->CurrentStatus, JobBackground);
			Parent->AddBuildText(this->CurrentStageName + " - failed", JobBackground);
		}
		else
		{
			Parent->AddBuildText(this->CurrentStageName + " - done", JobBackground);
		}
	}

	if (StageName.empty())
	{
		BuildDone();
		return;
	}

	CurrentStageName = StageName;

	Parent->AddBuildHeader(StageName, JobBackground);

	StatusText = new UIText(11_px, EditorUI::Theme.Text, "", Parent->DefaultFont);

	ProgressBox = (new UIBox(true, 0))
		->SetPadding(5_px, 5_px, 15_px, 15_px)
		->AddChild((new UISpinner(0, EditorUI::Theme.Highlight1, 20_px))
			->SetBackgroundColor(EditorUI::Theme.Background)
			->SetPadding(0, 30_px, 0, 20_px))
		->AddChild(StatusText
			->SetWrapEnabled(true, 300_px));

	JobBackground->AddChild(ProgressBox);
}

#include "ConsolePanel.h"
#include <Core/Log.h>
#include <Editor/UI/EditorUI.h>
#include <ConsolePanel.kui.hpp>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/Engine.h>
#include <kui/Window.h>
#include <Editor/Settings/EditorSettings.h>
#include <Editor/UI/Windows/SettingsWindow.h>

using namespace kui;
using namespace engine;

static std::map<Log::LogColor, Vec3f> LogColorValues =
{
	{Log::LogColor::Default, Vec3f(0.7f)},
	{Log::LogColor::White, Vec3f(1)},
	{Log::LogColor::Gray, Vec3f(0.7f)},
	{Log::LogColor::Red, Vec3f(1, 0.2f, 0.0f)},
	{Log::LogColor::Green, Vec3f(0.5f, 1.0f, 0.0f)},
	{Log::LogColor::Cyan, Vec3f(0, 1, 1)},
	{Log::LogColor::Magenta, Vec3f(1, 0.1f, 1)},
	{Log::LogColor::Blue, Vec3f(0.25f, 0.25f, 1)},
	{Log::LogColor::Yellow, Vec3f(1, 1, 0)},
};

static std::map<Log::LogColor, Vec3f> LogColorValuesLight =
{
	{Log::LogColor::Default, Vec3f(0.3f)},
	{Log::LogColor::White, Vec3f(0)},
	{Log::LogColor::Gray, Vec3f(0.3f)},
	{Log::LogColor::Red, Vec3f(1, 0.0f, 0.0f)},
	{Log::LogColor::Green, Vec3f(0.0f, 0.6f, 0.0f)},
	{Log::LogColor::Cyan, Vec3f(0, 0.6f, 0.6f)},
	{Log::LogColor::Magenta, Vec3f(0.6f, 0, 0.6f)},
	{Log::LogColor::Blue, Vec3f(0, 0, 0.6f)},
	{Log::LogColor::Yellow, Vec3f(0.6f, 0.6f, 0)},
};

engine::editor::ConsolePanel::ConsolePanel()
	: EditorPanel("Console", "ConsolePanel")
{
	Element = new ConsolePanelElement();
	Element->commandField->field->OnChanged = [this]() {
		string Command = Element->commandField->field->GetText();

		if (Command.empty() || !Element->GetParentWindow()->Input.IsKeyDown(Key::RETURN))
			return;

		Log::Info("> " + Command);
		Element->commandField->field->SetText("");

		auto sys = Engine::GetSubsystem<ConsoleSubsystem>();

		if (sys)
		{
			sys->ExecuteCommand(Command);
		}
	};

	Element->searchField->field->OnValueChanged = [this]() {
		this->Filter = str::Lower(Element->searchField->field->GetText());
		UpdateLog(true);
	};

	Element->optionsButton->OnClicked = [this] {
		bool ClearConsole = Settings::GetInstance()->Console.GetSetting("clearLogWhenGameStarts", false).GetBool();
		bool VerboseLog = Settings::GetInstance()->Console.GetSetting("verboseLog", false).GetBool();

		new DropdownMenu(
			{
				DropdownMenu::Option("Clear log now", "", EditorUI::Asset("Reload.png"), &Log::Clear, {}, true),
				DropdownMenu::Option("Clear log when game starts", "", ClearConsole ? EditorUI::Asset("Check.png") : "",
					[ClearConsole] {
					Settings::GetInstance()->Console.SetSetting("clearLogWhenGameStarts", !ClearConsole);
				}),
				DropdownMenu::Option("Verbose log messages", "", VerboseLog ? EditorUI::Asset("Check.png") : "",
					[VerboseLog] {
					Settings::GetInstance()->Console.SetSetting("verboseLog", !VerboseLog);
				}),
				DropdownMenu::Option("Show all console settings", "", EditorUI::Asset("Open.png"),
					[] {
					new SettingsWindow("Console");
				})
			},
			Element->optionsButton->GetPosition() + SizeVec::Pixels(-5, 16).GetScreen(), true);
	};

	Background->AddChild(Element);
}

engine::editor::ConsolePanel::~ConsolePanel()
{
}

void engine::editor::ConsolePanel::Update()
{
	if (Log::GetLogMessagesCount() != LastLogSize)
	{
		UpdateLog(false);
	}
}

void engine::editor::ConsolePanel::OnResized()
{
	using namespace kui;

	Element->SetLogSize(Size - SizeVec(2_px, 38_px).GetScreen());
	UpdateLog(true);
}

void engine::editor::ConsolePanel::OnThemeChanged()
{
	UpdateLog(true);
}

void engine::editor::ConsolePanel::UpdateLog(bool Full)
{
	using namespace kui;

	std::vector LogMessages = Log::GetMessages();

	if (LogMessages.size() < LastLogSize)
	{
		Full = true;
	}

	if (Full)
	{
		Element->logBox->DeleteChildren();
		LastLogSize = 0;
	}

	auto& Children = Element->logBox->GetChildren();

	if (Children.size() > 0)
	{
		Children[Children.size() - 1]->SetPadding(0_px, 0_px, 5_px, 0_px);
	}

	auto* Colors = EditorUI::Theme.IsLight ? &LogColorValuesLight : &LogColorValues;

	for (size_t i = LastLogSize; i < LogMessages.size(); i++)
	{
		if (!Filter.empty() && str::Lower(LogMessages[i].Message).find(Filter) == string::npos)
		{
			continue;
		}

		std::vector<TextSegment> Segments;

		auto& DefaultColor = Colors->at(Log::LogColor::Default);

		for (const Log::LogPrefix& pref : LogMessages[i].Prefixes)
		{
			Segments.push_back(TextSegment("[", DefaultColor));
			Segments.push_back(TextSegment(pref.Text, Colors->at(pref.Color)));
			Segments.push_back(TextSegment("]: ", DefaultColor));
		}

		Segments.push_back(TextSegment(LogMessages[i].Message, Colors->at(LogMessages[i].Color)));

		UIText* txt = new UIText(11_px, Segments, EditorUI::MonospaceFont);

		txt->SetWrapEnabled(true, Size.X - (40_px).GetScreen().X);

		txt
			->SetPadding(0_px, 0_px, 5_px, 0_px);

		Element->logBox->AddChild(txt);
	}

	if (!Children.empty())
	{
		Children[Children.size() - 1]->SetPadding(0_px, 15_px, 5_px, 0_px);
	}

	LastLogSize = LogMessages.size();
	PanelElement->UpdateElement();
	Element->logBox->GetScrollObject()->Scrolled = Element->logBox->GetScrollObject()->MaxScroll;
	Element->logBox->RedrawElement();
}

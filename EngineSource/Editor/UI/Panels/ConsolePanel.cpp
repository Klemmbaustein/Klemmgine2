#include "ConsolePanel.h"
#include <Core/Log.h>
#include <Editor/UI/EditorUI.h>
#include <ConsolePanel.kui.hpp>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/Engine.h>
#include <kui/Window.h>
#include <iostream>
using namespace engine;

static std::map<Log::LogColor, kui::Vec3f> LogColorValues =
{
	{Log::LogColor::Default, kui::Vec3f(0.7f)},
	{Log::LogColor::White, kui::Vec3f(1)},
	{Log::LogColor::Gray, kui::Vec3f(0.7f)},
	{Log::LogColor::Red, kui::Vec3f(1, 0.2f, 0.0f)},
	{Log::LogColor::Green, kui::Vec3f(0.5f, 1.0f, 0.0f)},
	{Log::LogColor::Cyan, kui::Vec3f(0, 1, 1)},
	{Log::LogColor::Magenta, kui::Vec3f(1, 0.1f, 1)},
	{Log::LogColor::Blue, kui::Vec3f(0.25f, 0.25f, 1)},
	{Log::LogColor::Yellow, kui::Vec3f(1, 1, 0)},
};

static std::map<Log::LogColor, kui::Vec3f> LogColorValuesLight =
{
	{Log::LogColor::Default, kui::Vec3f(0.3f)},
	{Log::LogColor::White, kui::Vec3f(0)},
	{Log::LogColor::Gray, kui::Vec3f(0.3f)},
	{Log::LogColor::Red, kui::Vec3f(1, 0.0f, 0.0f)},
	{Log::LogColor::Green, kui::Vec3f(0.0f, 0.6f, 0.0f)},
	{Log::LogColor::Cyan, kui::Vec3f(0, 0.6f, 0.6f)},
	{Log::LogColor::Magenta, kui::Vec3f(0.6f, 0, 0.6f)},
	{Log::LogColor::Blue, kui::Vec3f(0, 0, 0.6f)},
	{Log::LogColor::Yellow, kui::Vec3f(0.6f, 0.6f, 0)},
};

engine::editor::ConsolePanel::ConsolePanel()
	: EditorPanel("Console", "ConsolePanel")
{
	Element = new ConsolePanelElement();
	Element->commandField->field->OnChanged = [this]() {
		string Command = Element->commandField->field->GetText();

		if (Command.empty() || !Element->GetParentWindow()->Input.IsKeyDown(kui::Key::RETURN))
			return;

		Log::Info("> " + Command);
		Element->commandField->field->SetText("");
		Engine::GetSubsystem<ConsoleSubsystem>()->ExecuteCommand(Command);
	};

	Element->searchField->field->OnValueChanged = [this]() {
		this->Filter = str::Lower(Element->searchField->field->GetText());
		UpdateLog(true);
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

	if (Full)
	{
		Element->logBox->DeleteChildren();
		LastLogSize = 0;
	}
	std::vector LogMessages = Log::GetMessages();

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

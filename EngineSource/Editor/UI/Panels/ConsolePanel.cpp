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
	std::pair(Log::LogColor::Default, kui::Vec3f(0.7f)),
	std::pair(Log::LogColor::White, kui::Vec3f(1)),
	std::pair(Log::LogColor::Gray, kui::Vec3f(0.7f)),
	std::pair(Log::LogColor::Red, kui::Vec3f(1, 0.2f, 0.0f)),
	std::pair(Log::LogColor::Green, kui::Vec3f(0.5f, 1.0f, 0.0f)),
	std::pair(Log::LogColor::Cyan, kui::Vec3f(0, 1, 1)),
	std::pair(Log::LogColor::Magenta, kui::Vec3f(1, 0.1f, 1)),
	std::pair(Log::LogColor::Blue, kui::Vec3f(0.25f, 0.25f, 1)),
	std::pair(Log::LogColor::Yellow, kui::Vec3f(1, 1, 0)),
};

static std::map<Log::LogColor, kui::Vec3f> LogColorValuesLight =
{
	std::pair(Log::LogColor::Default, kui::Vec3f(0.3f)),
	std::pair(Log::LogColor::White, kui::Vec3f(0)),
	std::pair(Log::LogColor::Gray, kui::Vec3f(0.3f)),
	std::pair(Log::LogColor::Red, kui::Vec3f(1, 0.0f, 0.0f)),
	std::pair(Log::LogColor::Green, kui::Vec3f(0.0f, 0.6f, 0.0f)),
	std::pair(Log::LogColor::Cyan, kui::Vec3f(0, 0.6f, 0.6f)),
	std::pair(Log::LogColor::Magenta, kui::Vec3f(0.6f, 0, 0.6f)),
	std::pair(Log::LogColor::Blue, kui::Vec3f(0, 0, 0.6f)),
	std::pair(Log::LogColor::Yellow, kui::Vec3f(0.6f, 0.6f, 0)),
};

engine::editor::ConsolePanel::ConsolePanel()
	: EditorPanel("Console", "console")
{

	Element = new ConsolePanelElement();
	Element->commandField->field->OnChanged = [this]() {
		using namespace subsystem;
		string Command = Element->commandField->field->GetText();

		if (Command.empty() || !Element->GetParentWindow()->Input.IsKeyDown(kui::Key::RETURN))
			return;

		Log::Info("> " + Command);
		Element->commandField->field->SetText("");
		Engine::GetSubsystem<ConsoleSubsystem>()->ExecuteCommand(Command);
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

	Element->SetLogSize(Size - UIBox::PixelSizeToScreenSize(Vec2f(2, 34), Element->GetParentWindow()));
	UpdateLog(false);
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

	if (LastLogSize > 0)
	{
		Element->logBox->GetChildren()[LastLogSize - 1]->SetPadding(0_px, 0_px, 5_px, 0_px);
	}

	auto* Colors = EditorUI::Theme.IsLight ? &LogColorValuesLight : &LogColorValues;

	for (size_t i = LastLogSize; i < LogMessages.size(); i++)
	{
		std::vector<TextSegment> Segments;

		for (const Log::LogPrefix& i : LogMessages[i].Prefixes)
		{
			Segments.push_back(TextSegment("[", Colors->at(Log::LogColor::Default)));
			Segments.push_back(TextSegment(i.Text, Colors->at(i.Color)));
			Segments.push_back(TextSegment("]: ", Colors->at(Log::LogColor::Default)));
		}

		Segments.push_back(TextSegment(LogMessages[i].Message, Colors->at(LogMessages[i].Color)));

		UIText* txt = new UIText(11_px, Segments, EditorUI::MonospaceFont);

		txt
			->SetPadding(0_px, 0_px, 5_px, 0_px);

		Element->logBox->AddChild(txt);
	}

	if (LastLogSize > 0)
	{
		Element->logBox->GetChildren()[LogMessages.size() - 1]->SetPadding(0_px, 15_px, 5_px, 0_px);
	}

	LastLogSize = LogMessages.size();
	PanelElement->UpdateElement();
	Element->logBox->GetScrollObject()->Scrolled = Element->logBox->GetScrollObject()->MaxScroll;
	Element->logBox->RedrawElement();
}

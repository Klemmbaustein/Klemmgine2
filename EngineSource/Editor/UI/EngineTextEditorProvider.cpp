#include "EngineTextEditorProvider.h"
#include <kui/UI/UITextEditor.h>
#include <kui/Window.h>
#include <sstream>
#include <Editor/UI/EditorUI.h>
#include <kui/UI/UIBlurBackground.h>
#include <Engine/File/Resource.h>
#include <Core/Log.h>

using namespace ds;
using namespace kui;
using namespace engine::editor;

engine::editor::EngineTextEditorProvider::EngineTextEditorProvider(std::string File)
{
	this->EditedFile = File;

	std::stringstream Stream;
	Stream << resource::GetTextFile(File);

	LoadStream(Stream);
}

engine::editor::EngineTextEditorProvider::~EngineTextEditorProvider()
{
}

void engine::editor::EngineTextEditorProvider::Update()
{
	auto Win = Window::GetActiveWindow();

	auto Pos = Win->Input.MousePosition;

	if (Win->Input.IsRMBClicked)
	{
		std::vector<DropdownMenu::Option> Options = GetRightClickOptions(ParentEditor->ScreenToEditor(Pos, false));

		new DropdownMenu(Options, Pos);

		if (ParentEditor->GetSelectedText().empty())
		{
			ParentEditor->SetCursorPosition(ParentEditor->ScreenToEditor(Pos));
		}
		delete this->HoverBox;
		this->HoverBox = nullptr;
		OnRightClick();
	}
}

void engine::editor::EngineTextEditorProvider::SetLine(size_t Index, const std::vector<kui::TextSegment>& NewLine)
{
	FileEditorProvider::SetLine(Index, NewLine);
	if (Index == ParentEditor->SelectionEnd.Line)
	{
		auto& Input = Window::GetActiveWindow()->Input;
		auto Filter = this->Lines[Index].substr(0, ParentEditor->SelectionEnd.Column + 1);
		string LastWord;
		if (!Filter.empty())
		{
			size_t LastCharAfterSpace = Filter.find_last_of("\t .()-+/*><=") + 1;
			LastWord = Filter.substr(LastCharAfterSpace);
		}

		if (Input.Text == ".")
		{
			ShowAutoComplete(CompletionSource::TriggerChar);
		}
		else if (IsAutoCompleteActive && LastWord.size())
		{
			UpdateAutoCompleteEntries(LastWord);
		}
		else if (IsAutoCompleteActive && (Input.Text.size() > 1 || !std::isalpha(Input.Text[0])))
		{
			CloseAutoComplete();
		}
		else if (Input.Text.size() == 1 && std::isalpha(Input.Text[0]))
		{
			ShowAutoComplete(CompletionSource::WordCompletion, LastWord);
		}
	}

}

std::string engine::editor::EngineTextEditorProvider::ProcessInput(std::string Text)
{
	if (IsAutoCompleteActive && CompletionButtons.size() && Text == "\t")
	{
		CompletionButtons[0]->OnButtonClicked();
		return "";
	}
	return Text;
}

std::vector<ds::AutoCompleteResult> engine::editor::EngineTextEditorProvider::GetCompletionsAt(
	kui::EditorPosition At, CompletionSource Source)
{
	return {};
}

void engine::editor::EngineTextEditorProvider::UpdateAutoComplete()
{
	auto& Input = Window::GetActiveWindow()->Input;
	auto& UI = Window::GetActiveWindow()->UI;

	if (IsAutoCompleteActive && (Input.IsKeyDown(Key::ESCAPE) || (Input.IsLMBDown && UI.HoveredBox != HoverBox)))
	{
		CloseAutoComplete();
		ParentEditor->Edit();
	}
	if (!IsAutoCompleteActive && Input.IsKeyDown(Key::SPACE) && Input.IsKeyDown(Key::CTRL))
	{
		ShowAutoComplete(CompletionSource::Shortcut);
	}
}

void engine::editor::EngineTextEditorProvider::InsertCompletion(const AutoCompleteResult& Result)
{
	auto Position = ParentEditor->GetCursorPosition();
	string Line = this->Lines[Position.Line];
	string LineEnd = Line.substr(Position.Column);
	Line = Line.substr(0, Position.Column);
	if (!Line.empty())
	{
		size_t LastCharAfterSpace = Line.find_last_of("\t .()-+/*><=") + 1;
		Line = Line.substr(0, LastCharAfterSpace);
	}
	Position.Column = Line.size() + Result.name.size();

	SetLine(Position.Line, { TextSegment(Line + Result.name + LineEnd, Vec3f(1)) });

	if (!Result.completionModule.empty())
	{
		auto& ln = ParentEditor->GetLine(GetCompletionUsingLine());

		string Text = TextSegment::CombineToString(ln.Data);

		size_t LastWhitespace = Text.find_first_not_of(" \t");
		Text = Text.substr(0, LastWhitespace);

		ParentEditor->Insert(Text + "using " + Result.completionModule + "\n",
			EditorPosition(0, GetCompletionUsingLine()), true, false);
		Position.Line++;
	}

	ParentEditor->SetCursorPosition(Position);
	Commit();
}

UIBox* engine::editor::EngineTextEditorProvider::CreateHoverBox(kui::UIBox* Content, EditorPosition At)
{
	ClearHovered();

	this->HoverBox = (new UIBlurBackground(true, 0, EditorUI::Theme.LightBackground))
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	if (Content)
	{
		this->HoverBox
			->AddChild(Content);
	}
	this->HoverBox->HasMouseCollision = true;

	ApplyHoverBoxPosition(HoverBox, At);

	return this->HoverBox;
}

void engine::editor::EngineTextEditorProvider::ApplyHoverBoxPosition(kui::UIBox* Target, EditorPosition At)
{
	this->HoverBox->UpdateElement();

	this->HoverBox->SetPosition(ParentEditor->EditorToScreen(At)
		+ Vec2f(0, ParentEditor->EditorScrollBox->GetScrollObject()->GetOffset())
		- Vec2f(0, this->HoverBox->GetUsedSize().GetScreen().Y) + Vec2f(0, (1_px).GetScreen().Y));
}

void engine::editor::EngineTextEditorProvider::ClearHovered()
{
	if (this->HoverBox)
	{
		delete this->HoverBox;
		this->HoverBox = nullptr;
	}
}

void engine::editor::EngineTextEditorProvider::UpdateAutoCompleteEntries(string Filter)
{
	AutoCompleteBox->DeleteChildren();
	CompletionButtons.clear();
	HoverBox->IsVisible = false;
	for (auto& i : Completions)
	{
		size_t Found = string::npos;

		if (!Filter.empty())
		{
			Found = i.name.find(Filter);
			if (Found == string::npos)
			{
				continue;
			}
		}
		else if (!i.completionModule.empty())
		{
			continue;
		}

		HoverBox->IsVisible = true;

		auto btn = new UIButton(true, 0, EditorUI::Theme.LightBackground, [this, i = i]() {
			ParentEditor->Edit();
			InsertCompletion(i);
			CloseAutoComplete();
		});

		CompletionButtons.push_back(btn);

		btn->OnlyDrawWhenHovered = true;

		std::vector<TextSegment> Segments;

		if (Found == string::npos)
		{
			Segments.push_back({ TextSegment{ i.name, EditorUI::Theme.Text } });
		}
		else
		{
			Segments.push_back({ TextSegment{ i.name.substr(0, Found), EditorUI::Theme.Text} });
			Segments.push_back({ TextSegment{ i.name.substr(Found, Filter.size()), EditorUI::Theme.HighlightText} });
			Segments.push_back({ TextSegment{ i.name.substr(Found + Filter.size()), EditorUI::Theme.Text} });
		}
		if (i.type == CompletionType::function || i.type == CompletionType::method)
		{
			Segments.push_back({ TextSegment{ "()", EditorUI::Theme.DarkText} });
		}

		AutoCompleteBox->AddChild(btn
			->SetMinWidth(UISize::Parent(1))
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(12_px, Segments, EditorUI::EditorFont))
				->SetTextWidthOverride(150_px)
				->SetPadding(3_px)));

		if (!i.completionModule.empty())
		{
			btn->AddChild((new UIBox(true))
				->SetMinWidth(80_px)
				->SetPadding(2_px)
				->SetHorizontalAlign(UIBox::Align::Reverse)
				->AddChild((new UIText(11_px,
					{ TextSegment{ i.completionModule, EditorUI::Theme.DarkText} },
					EditorUI::EditorFont))
					->SetWrapEnabled(true, 80_px)));
		}
	}

	ApplyHoverBoxPosition(HoverBox, CompletePosition);
	HoverBox->UpdateElement();
	AutoCompleteBox->Update();
}

void engine::editor::EngineTextEditorProvider::CloseAutoComplete()
{
	Completions.clear();
	CompletionButtons.clear();
	delete HoverBox;
	HoverBox = nullptr;
	AutoCompleteBox = nullptr;
	IsAutoCompleteActive = false;
}

void engine::editor::EngineTextEditorProvider::ShowAutoComplete(CompletionSource Source, std::string Filter)
{
	CompletePosition = ParentEditor->SelectionStart;

	Completions = GetCompletionsAt(CompletePosition, Source);

	if (Completions.empty())
	{
		return;
	}

	auto box = CreateHoverBox(nullptr, CompletePosition);
	AutoCompleteBox = new UIScrollBox(false, 0, true);
	box->AddChild(AutoCompleteBox);
	AutoCompleteBox->GetScrollBarBackground()->SetOpacity(0);
	AutoCompleteBox->GetScrollBarSlider()->SetOpacity(0.75f);

	AutoCompleteBox->SetMinSize(SizeVec(250_px, 0));
	AutoCompleteBox->SetMaxSize(SizeVec(250_px, 200_px));
	AutoCompleteBox->SetPadding(3_px);

	UpdateAutoCompleteEntries(Filter);

	IsAutoCompleteActive = true;
}

void engine::editor::EngineTextEditorProvider::TrimWhitespace(size_t IgnoreLine)
{
	bool Changed = false;
	for (size_t i = 0; i < Lines.size(); i++)
	{
		if (IgnoreLine == i)
		{
			continue;
		}

		auto& str = Lines[i];

		auto lastChar = str.find_last_not_of("\t ");

		if (lastChar == str.size())
		{
			continue;
		}

		auto sub = str.substr(0, lastChar + 1);

		if (ParentEditor->IsLineLoaded(i))
		{
			SetLine(i, { TextSegment(sub, TextColor) });
		}
		else
		{
			this->Lines[i] = sub;
		}
		Changed = true;
	}

	Commit();
}

std::vector<DropdownMenu::Option> engine::editor::EngineTextEditorProvider::GetRightClickOptions(EditorPosition At)
{
	std::vector<DropdownMenu::Option> Options;

	Options.push_back(DropdownMenu::Option{
		.Name = "Cut",
		.Shortcut = "Ctrl+X",
		.OnClicked = [this] {
			Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
			ParentEditor->DeleteSelection();
	} });

	Options.push_back(DropdownMenu::Option{
		.Name = "Copy",
		.Shortcut = "Ctrl+C",
		.OnClicked = [this] {
			Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
	} });

	Options.push_back(DropdownMenu::Option{
		.Name = "Paste",
		.Shortcut = "Ctrl+V",
		.OnClicked = [this] {
			ParentEditor->DeleteSelection();
			ParentEditor->Insert(Window::GetActiveWindow()->Input.GetClipboard(),
				ParentEditor->GetCursorPosition(), true);
		}
		});
	return Options;
}

void engine::editor::EngineTextEditorProvider::OnRightClick()
{
}

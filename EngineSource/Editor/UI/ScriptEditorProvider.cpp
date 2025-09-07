#include "ScriptEditorProvider.h"
#include <Engine/Engine.h>
#include <kui/UI/UITextEditor.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <modules/standardLibrary.hpp>
#include <service/languageService.hpp>
#include <Engine/Input.h>
#include <Editor/UI/DropdownMenu.h>
#include <algorithm>
#include <kui/Window.h>
#include <Editor/UI/EditorUI.h>

using namespace kui;
using namespace lang;

engine::editor::ScriptEditorProvider::ScriptEditorProvider(std::string ScriptFile)
	: FileEditorProvider(ScriptFile)
{
	this->ScriptFile = ScriptFile;
	this->ScriptService = Engine::GetSubsystem<script::ScriptSubsystem>()
		->ScriptLanguage->startService();

	this->ScriptService->addString(this->GetContent(), ScriptFile);

	this->ScriptService->parser->errors.errorCallback = [this]
	(ErrorCode code, std::string File, const Token& At, std::string Description)
	{
		this->Errors.push_back(ScriptError{
			.At = EditorPosition(At.position.startPos, At.position.line),
			.Length = At.position.endPos - At.position.startPos,
			.Description = Description,
			});
	};

	ScanFile();
}

void engine::editor::ScriptEditorProvider::GetHighlightsForRange(size_t Begin, size_t Length)
{
	FileEditorProvider::GetHighlightsForRange(Begin, Length);

	for (auto& i : Errors)
	{
		ParentEditor->HighlightArea(HighlightedArea{
			.Start = i.At,
			.End = EditorPosition(i.At.Column + i.Length, i.At.Line),
			.Color = Vec3f(1.0f, 0, 0.2f),
			.Priority = 10,
			.Size = 2_px,
			});
	}
}

void engine::editor::ScriptEditorProvider::RemoveLines(size_t Start, size_t Length)
{
	FileEditorProvider::RemoveLines(Start, Length);
	UpdateFile();
}

void engine::editor::ScriptEditorProvider::SetLine(size_t Index, const std::vector<TextSegment>& NewLine)
{
	FileEditorProvider::SetLine(Index, NewLine);
	UpdateFile();
	UpdateLineColorization(Index);
}

void engine::editor::ScriptEditorProvider::InsertLine(size_t Index, const std::vector<TextSegment>& Content)
{
	FileEditorProvider::InsertLine(Index, Content);
	UpdateFile();
	UpdateLineColorization(Index);
}

void engine::editor::ScriptEditorProvider::GetLine(size_t LineIndex, std::vector<TextSegment>& To)
{
	FileEditorProvider::GetLine(LineIndex, To);
	UpdateLineColorization(LineIndex);
}

void engine::editor::ScriptEditorProvider::OnLoaded()
{
}

void engine::editor::ScriptEditorProvider::Update()
{
	auto Window = Window::GetActiveWindow();
	auto Pos = Window->Input.MousePosition;
	if (input::IsRMBClicked)
	{
		new DropdownMenu({
			DropdownMenu::Option{
				.Name = "Cut",
			},
			DropdownMenu::Option{
				.Name = "Copy",
			},
			DropdownMenu::Option{
				.Name = "Paste",
				.OnClicked = [this] {
					ParentEditor->DeleteSelection();
					ParentEditor->Insert(Window::GetActiveWindow()->Input.GetClipboard(),
						ParentEditor->GetCursorPosition(), true);
				}
			},
			},
			Pos);

		if (ParentEditor->GetSelectedText().empty())
		{
			ParentEditor->SetCursorPosition(ParentEditor->ScreenToEditor(Pos));
		}
	}

	if (Window->Input.MousePosition == LastCursorPosition)
	{
		if (HoverTime.Get() > 0.1f && !this->HoveredBox)
		{
			auto HoverPosition = ParentEditor->ScreenToEditor(Pos);

			for (auto& i : this->Errors)
			{
				if (HoverPosition.Line == i.At.Line
					&& (HoverPosition.Column >= i.At.Column && HoverPosition.Column <= i.Length + i.At.Column))
				{
					CreateHoverBox((new UIText(12_px, 1, i.Description, EditorUI::MonospaceFont))
						->SetPadding(5_px), HoverPosition);
					break;
				}
			}

			//for (auto& i : this->ScriptService->files.begin()->second.functions)
			//{
			//	if (HoverPosition.Line == i.position.line
			//		&& (HoverPosition.Column >= i.position.startPos && HoverPosition.Column <= i.position.endPos))
			//	{
			//		CreateHoverBox((new UIText(12_px, 1, i.string, EditorUI::EditorFont))
			//			->SetPadding(5_px), HoverPosition);
			//		break;
			//	}
			//}
		}
	}
	else if (this->HoveredBox)
	{
		HoverTime.Reset();
		delete this->HoveredBox;
		this->HoveredBox = nullptr;
	}
	LastCursorPosition = Window->Input.MousePosition;
}

UIBox* engine::editor::ScriptEditorProvider::CreateHoverBox(kui::UIBox* Content, EditorPosition At)
{
	if (this->HoveredBox)
	{
		delete this->HoveredBox;
	}

	this->HoveredBox = new UIBackground(true, 0, 0.2f);
	this->HoveredBox->AddChild(Content);
	this->HoveredBox->UpdateElement();

	this->HoveredBox->SetPosition(ParentEditor->EditorToScreen(At)
		- Vec2f(0, this->HoveredBox->GetUsedSize().GetScreen().Y));
	return this->HoveredBox;
}

void engine::editor::ScriptEditorProvider::UpdateLineColorization(size_t Line)
{
	auto found = Highlights.find(Line);

	if (found == Highlights.end())
	{
		return;
	}

	std::sort(found->second.begin(), found->second.end(), []
	(const ScriptSyntaxHighlight& a, const ScriptSyntaxHighlight& b) {
		return a.Start < b.Start;
	});

	std::vector<EditorColorizeSegment> Segments;
	size_t LastStart = 0;

	for (ScriptSyntaxHighlight& i : found->second)
	{
		Segments.push_back(EditorColorizeSegment{
			.Offset = i.Start - LastStart,
			.Length = i.Length,
			.Color = i.Color,
			});
		LastStart = i.Start + i.Length;
	}

	ParentEditor->ColorizeLine(Line, Segments);
}

void engine::editor::ScriptEditorProvider::UpdateFile()
{
	Highlights.clear();
	Errors.clear();
	this->ScriptService->updateFile(this->GetContent(), this->ScriptFile);
	ScanFile();
}

void engine::editor::ScriptEditorProvider::ScanFile()
{
	this->ScriptService->commitChanges();

	if (this->ScriptService->files.size())
	{
		for (auto& i : this->ScriptService->files.begin()->second.functions)
		{
			size_t ActualStart = i.position.startPos;

			size_t Colon = i.string.find_last_of(':');

			if (Colon != std::string::npos)
			{
				ActualStart += Colon + 1;
			}

			Highlights[i.position.line].push_back(ScriptSyntaxHighlight{
				.Start = ActualStart,
				.Length = i.position.endPos - ActualStart,
				.Color = FunctionColor,
				});
		}

		for (auto& i : this->ScriptService->files.begin()->second.types)
		{
			size_t ActualStart = i.position.startPos;

			size_t Colon = i.string.find_last_of(':');

			if (Colon != std::string::npos)
			{
				ActualStart += Colon + 1;
			}

			Highlights[i.position.line].push_back(ScriptSyntaxHighlight{
				.Start = ActualStart,
				.Length = i.position.endPos - ActualStart,
				.Color = TypeColor,
				});
		}

		for (auto& i : this->ScriptService->files.begin()->second.variables)
		{
			Highlights[i.position.line].push_back(ScriptSyntaxHighlight{
				.Start = i.position.startPos,
				.Length = i.position.endPos - i.position.startPos,
				.Color = VariableColor,
				});
		}
	}
}

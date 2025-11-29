#include "ScriptEditorProvider.h"
#include <Engine/Engine.h>
#include <kui/UI/UITextEditor.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/service/languageService.hpp>
#include <Engine/Input.h>
#include <Engine/MainThread.h>
#include <Editor/UI/DropdownMenu.h>
#include <algorithm>
#include <kui/Window.h>
#include <kui/UI/UIBlurBackground.h>
#include <Editor/UI/EditorUI.h>

using namespace kui;
using namespace ds;
using namespace engine::editor;

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
	NextChange.Parts.push_back(ChangePart{
		.Line = Start,
		.Content = Lines[Start],
		.IsRemove = true,
		});
	FileEditorProvider::RemoveLines(Start, Length);

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "delete"),
			SerializedData("file", this->ScriptFile),
			SerializedData("line", int32(Start)),
			SerializedData("length", int32(Length)),
			}));
	}
}

void engine::editor::ScriptEditorProvider::InsertLine(size_t Index, const std::vector<TextSegment>& Content)
{
	NextChange.Parts.push_back(ChangePart{
		.Line = Index,
		.IsAdd = true,
		});
	FileEditorProvider::InsertLine(Index, Content);

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "insert"),
			SerializedData("file", this->ScriptFile),
			SerializedData("line", int32(Index)),
			SerializedData("newContent", TextSegment::CombineToString(Content)),
			}));
	}
}

void engine::editor::ScriptEditorProvider::SetLine(size_t Index, const std::vector<TextSegment>& NewLine)
{
	NextChange.Parts.push_back(ChangePart{
		.Line = Index,
		.Content = this->Lines[Index],
		});
	UnDoneChanges = {};

	FileEditorProvider::SetLine(Index, NewLine);

	Changed.push_back(Index);

	auto& Input = Window::GetActiveWindow()->Input;

	if (Input.Text == ".")
	{
		ShowAutoComplete();
	}

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "modify"),
			SerializedData("file", this->ScriptFile),
			SerializedData("line", int32(Index)),
			SerializedData("newContent", TextSegment::CombineToString(NewLine)),
			}));
	}
}

void engine::editor::ScriptEditorProvider::Commit()
{
	UpdateFile();
	if (NextChange.Parts.size())
	{
		this->Changes.push(NextChange);
	}
	for (size_t Index : Changed)
	{
		UpdateLineColorization(Index);
	}
	Changed.clear();

	NextChange = Change();
}

void engine::editor::ScriptEditorProvider::RefreshAll()
{
	UpdateSyntaxHighlight();
	ParentEditor->FullRefresh();
}

void engine::editor::ScriptEditorProvider::Undo()
{
	if (Changes.empty())
	{
		return;
	}

	auto& c = Changes.top();

	UnDoneChanges.push(ApplyChange(c));
	Changes.pop();
	Commit();
}

void engine::editor::ScriptEditorProvider::Redo()
{
	if (UnDoneChanges.empty())
	{
		return;
	}

	auto& c = UnDoneChanges.top();

	Changes.push(ApplyChange(c));
	UnDoneChanges.pop();
	Commit();
}

ScriptEditorProvider::Change engine::editor::ScriptEditorProvider::ApplyChange(const Change& Target)
{
	Change DoneChanges;

	for (auto p = Target.Parts.rbegin(); p < Target.Parts.rend(); p++)
	{
		if (p->IsAdd)
		{
			DoneChanges.Parts.push_back(ChangePart{
				.Line = p->Line,
				.Content = this->Lines[p->Line],
				.IsRemove = true,
				});
			ParentEditor->RemoveLine(p->Line);
		}
		else if (p->IsRemove)
		{
			DoneChanges.Parts.push_back(ChangePart{
				.Line = p->Line,
				.IsAdd = true,
				});
			std::vector<TextSegment> s = { TextSegment(p->Content, 1) };
			ParentEditor->AddLine(p->Line, s);
		}
		else
		{
			DoneChanges.Parts.push_back(ChangePart{
				.Line = p->Line,
				.Content = this->Lines[p->Line],
				});
			std::vector<TextSegment> s = { TextSegment(p->Content, 1) };
			FileEditorProvider::SetLine(p->Line, s);
			Changed.push_back(p->Line);
			this->Lines[p->Line] = p->Content;
		}
	}

	NextChange = {};

	Log::Info(str::Format("Applied change with %i part(s)", Target.Parts.size()));

	return DoneChanges;
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
	auto Win = Window::GetActiveWindow();

	auto Pos = Win->Input.MousePosition;

	HoverErrorData NewHoveredError = GetHoveredError(Pos);
	HoverSymbolData NewHoveredSymbol = GetHoveredSymbol(Pos);

	void* NewHovered = NewHoveredError.Error ? (void*)NewHoveredError.Error : (void*)NewHoveredSymbol.Symbol;

	if (DropdownMenu::Current || !Win->UI.HoveredBox || !Win->UI.HoveredBox->IsChildOf(ParentEditor))
	{
		NewHovered = nullptr;
	}

	float Time = HoverTime.Get();

	UpdateAutoComplete();

	if (NewHovered != this->HoveredData || (Time > 0.2f && !HoveredBox && HoveredData))
	{
		if (NewHovered != this->HoveredData)
		{
			HoverTime.Reset();
			this->HoveredData = NewHovered;
			Time = 0;
			delete this->HoveredBox;
			this->HoveredBox = nullptr;
		}

		if (NewHovered && Time > 0.2f)
		{
			if (NewHoveredError.Error)
			{
				CreateHoverBox((new UIText(12_px, 1, NewHoveredError.Error->Description, EditorUI::MonospaceFont))
					->SetPadding(5_px), NewHoveredError.At);
			}
			else
			{
				CreateHoverBox(NewHoveredSymbol.GetHoverData(), NewHoveredSymbol.At);
			}
		}
		else if (!NewHovered)
		{
			delete this->HoveredBox;
			this->HoveredBox = nullptr;
		}
	}

	if (!NewHovered)
	{
		HoverTime.Reset();
	}

	if (input::IsRMBClicked)
	{
		std::vector<DropdownMenu::Option> Options;

		if (NewHoveredSymbol.GetDefinition)
		{
			Options.push_back(DropdownMenu::Option{
				.Name = "Go to definition",
				.Icon = EditorUI::Asset("TabDrag.png"),
				.OnClicked = [this, NewHoveredSymbol]
				{
					auto def = NewHoveredSymbol.GetDefinition();
					ParentEditor->SetCursorPosition(EditorPosition(def.position.startPos, def.position.line),
						EditorPosition(def.position.endPos, def.position.line));
					ParentEditor->ScrollTo(ParentEditor->SelectionStart);
					ParentEditor->Edit();
				},
				.Separator = true,
				});
		}

		Options.push_back(DropdownMenu::Option{
			.Name = "Cut",
			.OnClicked = [this] {
				Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
				ParentEditor->DeleteSelection();
		} });

		Options.push_back(DropdownMenu::Option{
			.Name = "Copy",
			.OnClicked = [this] {
				Window::GetActiveWindow()->Input.SetClipboard(ParentEditor->GetSelectedText());
		} });

		Options.push_back(DropdownMenu::Option{
				.Name = "Paste",
				.OnClicked = [this] {
					ParentEditor->DeleteSelection();
					ParentEditor->Insert(Window::GetActiveWindow()->Input.GetClipboard(),
						ParentEditor->GetCursorPosition(), true);
				}
			});

		new DropdownMenu(Options, Pos);

		if (ParentEditor->GetSelectedText().empty())
		{
			ParentEditor->SetCursorPosition(ParentEditor->ScreenToEditor(Pos));
		}
		delete this->HoveredBox;
		this->HoveredBox = nullptr;
	}

	LastCursorPosition = Win->Input.MousePosition;
}

UIBox* engine::editor::ScriptEditorProvider::CreateHoverBox(kui::UIBox* Content, EditorPosition At)
{
	if (this->HoveredBox)
	{
		delete this->HoveredBox;
	}

	this->HoveredBox = (new UIBlurBackground(true, 0, EditorUI::Theme.LightBackground))
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	this->HoveredBox
		->AddChild(Content);
	this->HoveredBox->UpdateElement();
	this->HoveredBox->HasMouseCollision = true;
	this->HoveredBox->SetCurrentScrollObject(ParentEditor->EditorScrollBox->GetScrollObject());

	this->HoveredBox->SetPosition(ParentEditor->EditorToScreen(At)
		- Vec2f(0, this->HoveredBox->GetUsedSize().GetScreen().Y) + Vec2f(0, (1_px).GetScreen().Y));
	return this->HoveredBox;
}

void engine::editor::ScriptEditorProvider::UpdateAutoComplete()
{
	auto& Input = Window::GetActiveWindow()->Input;

	if (!Input.IsKeyDown(Key::SPACE) || !Input.IsKeyDown(Key::LCTRL) || HoveredBox)
	{
		return;
	}

	ShowAutoComplete();
}

void engine::editor::ScriptEditorProvider::ShowAutoComplete()
{
	auto pos = ParentEditor->SelectionStart;

	auto c = this->ScriptService->completeAt(&ScriptService->files.begin()->second, pos.Column, pos.Line);

	UIBox* content = new UIBox(false);

	for (auto& i : c)
	{
		content->AddChild((new UIText(12_px, EditorUI::Theme.Text, i.name, EditorUI::EditorFont))
			->SetPadding(3_px));
	}

	//if (c.size())
	//{
	//	ParentEditor->Insert(c[0].name, pos, false);
	//}

	auto box = CreateHoverBox(content, pos);
}

void engine::editor::ScriptEditorProvider::LoadRemoteFile()
{
	Connection->GetFile("Scripts/test.ds", [=](ReadOnlyBufferStream* Stream)
	{
		string Content = Stream->ReadString();
		thread::ExecuteOnMainThread([=]
		{
			string NextLine;

			this->Lines.clear();

			for (auto c : Content)
			{
				if (c == '\r')
				{
					continue;
				}
				if (c == '\n')
				{
					this->Lines.push_back(NextLine);
					NextLine.clear();
				}
				else
				{
					NextLine.push_back(c);
				}
			}

			this->Lines.push_back(NextLine);
			delete Stream;

			this->UpdateFile();
			this->UpdateBracketAreas();
			this->ParentEditor->FullRefresh();
		});
	});
}

ScriptEditorProvider::HoverErrorData engine::editor::ScriptEditorProvider::GetHoveredError(Vec2f ScreenPosition)
{
	auto HoverPosition = ParentEditor->ScreenToEditor(ScreenPosition);

	for (auto& i : this->Errors)
	{
		if (HoverPosition.Line == i.At.Line
			&& (HoverPosition.Column >= i.At.Column && HoverPosition.Column <= i.Length + i.At.Column))
		{
			return HoverErrorData{
				.Error = &i,
				.At = HoverPosition,
			};
		}
	}
	return HoverErrorData();
}
ScriptEditorProvider::HoverSymbolData engine::editor::ScriptEditorProvider::GetHoveredSymbol(kui::Vec2f ScreenPosition)
{
	auto HoverPosition = ParentEditor->ScreenToEditor(ScreenPosition);

	if (this->ScriptService->files.empty())
	{
		return HoverSymbolData();
	}

	for (auto& i : this->ScriptService->files.begin()->second.functions)
	{
		if (HoverPosition.Line == i.at.position.line
			&& (HoverPosition.Column >= i.at.position.startPos && HoverPosition.Column <= i.at.position.endPos))
		{
			auto Callback = [this, Fn = &i] {
				std::vector<TextSegment> HoverString = {
					TextSegment("fn ", this->KeywordColor),
					TextSegment(Fn->name, this->FunctionColor),
					TextSegment("(", this->TextColor)
				};

				for (auto it = Fn->arguments.begin(); it < Fn->arguments.end(); it++)
				{
					HoverString.push_back(TextSegment(it->first + " ", TypeColor));
					HoverString.push_back(TextSegment(it->second, VariableColor));
					if (Fn->arguments.end() != it + 1)
					{
						HoverString.push_back(TextSegment(", ", this->TextColor));
					}
				}
				HoverString.push_back(TextSegment(")", this->TextColor));

				if (!Fn->returnType.empty())
				{
					HoverString.push_back(TextSegment(" -> ", this->TextColor));
					HoverString.push_back(TextSegment(Fn->returnType, TypeColor));
				}

				HoverString.push_back(TextSegment(Fn->definition ?
					"\nDefined in " + Fn->definition->file->name
					+ " at line " + std::to_string(Fn->definition->at.position.line + 1)
					: "\nDefined in native code.", this->TextColor));

				return (new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 1000_px)
					->SetPadding(5_px);
			};

			std::function<Token()> GetDefinition;

			if (i.definition)
			{
				GetDefinition = [i] {
					return i.definition->at;
				};
			}

			return HoverSymbolData{
				.Symbol = &i,
				.At = HoverPosition,
				.GetHoverData = Callback,
				.GetDefinition = GetDefinition,
			};
		}
	}

	for (auto& i : this->ScriptService->files.begin()->second.variables)
	{
		if (HoverPosition.Line == i.at.position.line
			&& (HoverPosition.Column >= i.at.position.startPos && HoverPosition.Column <= i.at.position.endPos))
		{
			auto Callback = [this, i] {
				auto DefaultColor = EditorUI::Theme.Text;

				std::vector<TextSegment> HoverString = {
					TextSegment(i.kind == ScannedVariable::Kind::localVariable
						? "(local variable) " : "(member) ", DefaultColor),
					TextSegment(i.type + " ", this->TypeColor),
				};

				if (!i.inClass.empty())
				{
					HoverString.push_back(TextSegment(i.inClass, this->TypeColor));
					HoverString.push_back(TextSegment(".", DefaultColor));
				}

				HoverString.push_back(TextSegment(i.name, this->VariableColor));

				if (!i.defaultValue.empty())
				{
					HoverString.push_back(TextSegment(" = " + i.defaultValue, DefaultColor));
				}

				return (new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 1000_px)
					->SetPadding(5_px);
			};

			std::function<Token()> GetDefinition;

			if (i.definition && i.definition->at.position.line != 0 || i.definition->at.position.endPos != 0)
			{
				GetDefinition = [i] {
					return i.definition->at;
				};
			}

			return HoverSymbolData{
				.Symbol = &i,
				.At = HoverPosition,
				.GetHoverData = Callback,
				.GetDefinition = GetDefinition,
			};
		}
	}

	return HoverSymbolData();
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
	Errors.clear();
	this->ScriptService->updateFile(this->GetContent(), this->ScriptFile);
	ScanFile();
}

void engine::editor::ScriptEditorProvider::ScanFile()
{
	this->ScriptService->commitChanges();
	OnUpdated.Invoke();
	UpdateSyntaxHighlight();
}

void engine::editor::ScriptEditorProvider::UpdateSyntaxHighlight()
{
	Highlights.clear();
	if (!this->ScriptService->files.size())
	{
		return;
	}
	for (auto& i : this->ScriptService->files.begin()->second.functions)
	{
		size_t ActualStart = i.at.position.startPos;

		size_t Colon = i.at.string.find_last_of(':');

		if (Colon != std::string::npos)
		{
			ActualStart += Colon + 1;
		}

		Highlights[i.at.position.line].push_back(ScriptSyntaxHighlight{
			.Start = ActualStart,
			.Length = i.at.position.endPos - ActualStart,
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
		Highlights[i.at.position.line].push_back(ScriptSyntaxHighlight{
			.Start = i.at.position.startPos,
			.Length = i.at.position.endPos - i.at.position.startPos,
			.Color = VariableColor,
			});
	}
}

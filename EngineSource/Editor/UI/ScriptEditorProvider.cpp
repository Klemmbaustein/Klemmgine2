#include "ScriptEditorProvider.h"
#include <kui/UI/UITextEditor.h>
#include <ds/service/languageService.hpp>
#include <Engine/MainThread.h>
#include <Editor/UI/DropdownMenu.h>
#include <algorithm>
#include <kui/Window.h>
#include <kui/UI/UIBlurBackground.h>
#include <Editor/UI/EditorUI.h>

using namespace kui;
using namespace ds;
using namespace engine::editor;

engine::editor::ScriptEditorProvider::ScriptEditorProvider(std::string ScriptFile, ScriptEditorContext* Context)
	: EngineTextEditorProvider(ScriptFile)
{
	this->Context = Context;
	Context->AddFile(this->GetContent(), ScriptFile);

	Context->OnReady.Add(this, [this]() {
		ParentEditor->FullRefresh();
		UpdateFileData();
	});
}

engine::editor::ScriptEditorProvider::~ScriptEditorProvider()
{
	Context->OnReady.Remove(this);
}

void engine::editor::ScriptEditorProvider::GetHighlightsForRange(size_t Begin, size_t Length)
{
	FileEditorProvider::GetHighlightsForRange(Begin, Length);

	for (auto& i : Context->Errors[EditedFile])
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
	EngineTextEditorProvider::RemoveLines(Start, Length);

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "delete"),
			SerializedData("file", this->EditedFile),
			SerializedData("line", int32(Start)),
			SerializedData("length", int32(Length)),
			}));
	}
}

void engine::editor::ScriptEditorProvider::InsertLine(size_t Index, const std::vector<TextSegment>& Content)
{
	EngineTextEditorProvider::InsertLine(Index, Content);

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "insert"),
			SerializedData("file", this->EditedFile),
			SerializedData("line", int32(Index)),
			SerializedData("newContent", TextSegment::CombineToString(Content)),
			}));
	}
}

std::string engine::editor::ScriptEditorProvider::ProcessInput(std::string Text)
{
	if (IsAutoCompleteActive && CompletionButtons.size() && Text == "\t")
	{
		CompletionButtons[0]->OnButtonClicked();
		return "";
	}
	return Text;
}

void engine::editor::ScriptEditorProvider::RefreshAll()
{
	UpdateSyntaxHighlight();
	ParentEditor->FullRefresh();
}

void engine::editor::ScriptEditorProvider::Commit()
{
	EngineTextEditorProvider::Commit();
	UpdateFileContent();
	for (size_t Index : Changed)
	{
		UpdateLineColorization(Index);
	}
	Changed.clear();
}

void engine::editor::ScriptEditorProvider::SetLine(size_t Index, const std::vector<TextSegment>& NewLine)
{
	EngineTextEditorProvider::SetLine(Index, NewLine);
	Changed.push_back(Index);
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
			ShowAutoComplete(true);
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
			ShowAutoComplete(false, LastWord);
		}
	}

	if (Connection)
	{
		Connection->SendMessage("scriptEdit", SerializedValue({
			SerializedData("editType", "modify"),
			SerializedData("file", this->EditedFile),
			SerializedData("line", int32(Index)),
			SerializedData("newContent", TextSegment::CombineToString(NewLine)),
			}));
	}
}

void engine::editor::ScriptEditorProvider::InsertCompletion(string CompletionText)
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
	Position.Column = Line.size() + CompletionText.size();

	SetLine(Position.Line, { TextSegment(Line + CompletionText + LineEnd, Vec3f(1)) });
	ParentEditor->SetCursorPosition(Position);
	Commit();
}

void engine::editor::ScriptEditorProvider::GetLine(size_t LineIndex, std::vector<TextSegment>& To)
{
	FileEditorProvider::GetLine(LineIndex, To);
	UpdateLineColorization(LineIndex);
}

void engine::editor::ScriptEditorProvider::OnLoaded()
{
}

void engine::editor::ScriptEditorProvider::OnRightClick()
{
	delete this->HoveredBox;
	this->HoveredBox = nullptr;
}

std::vector<DropdownMenu::Option> engine::editor::ScriptEditorProvider::GetRightClickOptions(kui::EditorPosition At)
{
	auto Pos = Window::GetActiveWindow()->Input.MousePosition;

	auto Options = EngineTextEditorProvider::GetRightClickOptions(At);
	HoverSymbolData NewHoveredSymbol = GetHoveredSymbol(Pos);

	if (NewHoveredSymbol.GetDefinition)
	{
		Options.insert(Options.begin(), DropdownMenu::Option{
			.Name = "Go to definition",
			.Shortcut = "F12",
			.Icon = EditorUI::Asset("Open.png"),
			.OnClicked = [this, NewHoveredSymbol]
			{
				auto def = NewHoveredSymbol.GetDefinition();
				Context->NavigateTo(def.File, def.Token.position);
			},
			.Separator = true,
			});
	}

	return Options;
}

void engine::editor::ScriptEditorProvider::Update()
{
	EngineTextEditorProvider::Update();

	auto Win = Window::GetActiveWindow();

	auto Pos = Win->Input.MousePosition;

	HoverErrorData NewHoveredError = GetHoveredError(Pos);
	HoverSymbolData NewHoveredSymbol = GetHoveredSymbol(Pos);

	if (Win->Input.IsLMBDown)
	{
		NewHoveredError = {};
		NewHoveredSymbol = {};
	}

	void* NewHovered = NewHoveredError.Error ? (void*)NewHoveredError.Error : (void*)NewHoveredSymbol.Symbol;

	if (IsAutoCompleteActive || DropdownMenu::Current || !Win->UI.HoveredBox
		|| !Win->UI.HoveredBox->IsChildOf(ParentEditor))
	{
		NewHovered = nullptr;
	}

	float Time = HoverTime.Get();

	UpdateAutoComplete();

	if (NewHovered != this->HoveredData || (Time > 0.2f && !HoveredBox && HoveredData))
	{
		if (NewHovered != this->HoveredData && !IsAutoCompleteActive)
		{
			this->HoveredData = NewHovered;
			Time = 0;
			ClearHovered();
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
		else if (!NewHovered && !IsAutoCompleteActive)
		{
			ClearHovered();
		}
	}

	if (!NewHovered)
	{
		HoverTime.Reset();
	}

	LastCursorPosition = Win->Input.MousePosition;
}

void engine::editor::ScriptEditorProvider::ClearHovered()
{
	if (this->HoveredBox)
	{
		delete this->HoveredBox;
		this->HoveredBox = nullptr;
		HoverTime.Reset();
	}
}

void engine::editor::ScriptEditorProvider::NavigateTo(kui::EditorPosition Position)
{
	NavigateTo(Position, Position);
}

void engine::editor::ScriptEditorProvider::NavigateTo(kui::EditorPosition StartPosition,
	kui::EditorPosition EndPosition)
{
	ParentEditor->SetCursorPosition(StartPosition,
		EditorPosition(EndPosition));
	ParentEditor->ScrollTo(ParentEditor->SelectionStart);
	ParentEditor->Edit();
}

UIBox* engine::editor::ScriptEditorProvider::CreateHoverBox(kui::UIBox* Content, EditorPosition At)
{
	ClearHovered();

	this->HoveredBox = (new UIBlurBackground(true, 0, EditorUI::Theme.LightBackground))
		->SetCorner(EditorUI::Theme.CornerSize)
		->SetBorder(1_px, EditorUI::Theme.BackgroundHighlight);
	if (Content)
	{
		this->HoveredBox
			->AddChild(Content);
	}
	this->HoveredBox->HasMouseCollision = true;

	ApplyHoverBoxPosition(HoveredBox, At);

	return this->HoveredBox;
}

void engine::editor::ScriptEditorProvider::ApplyHoverBoxPosition(kui::UIBox* Target, EditorPosition At)
{
	this->HoveredBox->UpdateElement();

	this->HoveredBox->SetPosition(ParentEditor->EditorToScreen(At)
		+ Vec2f(0, ParentEditor->EditorScrollBox->GetScrollObject()->GetOffset())
		- Vec2f(0, this->HoveredBox->GetUsedSize().GetScreen().Y) + Vec2f(0, (1_px).GetScreen().Y));
}

void engine::editor::ScriptEditorProvider::UpdateAutoComplete()
{
	auto& Input = Window::GetActiveWindow()->Input;
	auto& UI = Window::GetActiveWindow()->UI;

	if (IsAutoCompleteActive && (Input.IsKeyDown(Key::ESCAPE) || (Input.IsLMBDown && UI.HoveredBox != HoveredBox)))
	{
		CloseAutoComplete();
		ParentEditor->Edit();
	}

	if (!Input.IsKeyDown(Key::SPACE) || !Input.IsKeyDown(Key::CTRL))
	{
		return;
	}

	ShowAutoComplete(false);
}

void engine::editor::ScriptEditorProvider::ShowAutoComplete(bool MembersOnly, std::string Filter)
{
	CompletePosition = ParentEditor->SelectionStart;

	Completions = Context->CompleteAt(EditedFile, CompletePosition.Column, CompletePosition.Line,
		MembersOnly ? CompletionType::classMembers : CompletionType::all);

	if (Completions.empty())
	{
		return;
	}

	auto box = CreateHoverBox(nullptr, CompletePosition);
	AutoCompleteBox = new UIScrollBox(false, 0, true);
	box->AddChild(AutoCompleteBox);
	AutoCompleteBox->GetScrollBarBackground()->SetOpacity(0);
	AutoCompleteBox->GetScrollBarSlider()->SetOpacity(0.75f);

	AutoCompleteBox->SetMinSize(SizeVec(150_px, 0));
	AutoCompleteBox->SetMaxSize(SizeVec(150_px, 200_px));
	AutoCompleteBox->SetPadding(3_px);

	UpdateAutoCompleteEntries(Filter);

	IsAutoCompleteActive = true;
}

void engine::editor::ScriptEditorProvider::UpdateAutoCompleteEntries(string Filter)
{
	AutoCompleteBox->DeleteChildren();
	CompletionButtons.clear();
	HoveredBox->IsVisible = false;
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
		HoveredBox->IsVisible = true;

		auto btn = new UIButton(true, 0, EditorUI::Theme.LightBackground, [this, i = i]() {
			ParentEditor->Edit();
			InsertCompletion(i.name);
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
			->AddChild((new UIText(12_px, Segments, EditorUI::EditorFont))
				->SetPadding(3_px)));
	}
	ApplyHoverBoxPosition(HoveredBox, CompletePosition);
}

void engine::editor::ScriptEditorProvider::CloseAutoComplete()
{
	CompletionButtons.clear();
	delete HoveredBox;
	HoveredBox = nullptr;
	AutoCompleteBox = nullptr;
	IsAutoCompleteActive = false;
}

void engine::editor::ScriptEditorProvider::LoadRemoteFile()
{
	Connection->GetFile("Scripts/test.ds", [this](ReadOnlyBufferStream* Stream) {
		string Content = Stream->ReadString();
		thread::ExecuteOnMainThread([this, Content, Stream] {
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

			this->UpdateFileContent();
			this->UpdateBracketAreas();
			this->ParentEditor->FullRefresh();
		});
	});
}

ScriptEditorProvider::HoverErrorData engine::editor::ScriptEditorProvider::GetHoveredError(Vec2f ScreenPosition)
{
	if (Context->ScriptServiceMutex.try_lock())
	{
		auto HoverPosition = ParentEditor->ScreenToEditor(ScreenPosition, false);

		for (auto& i : Context->Errors[EditedFile])
		{
			if (HoverPosition.Line == i.At.Line
				&& (HoverPosition.Column >= i.At.Column && HoverPosition.Column <= i.Length + i.At.Column))
			{
				Context->ScriptServiceMutex.unlock();
				return HoverErrorData{
					.Error = &i,
					.At = HoverPosition,
				};
			}
		}
		Context->ScriptServiceMutex.unlock();
	}
	return HoverErrorData();
}
ScriptEditorProvider::HoverSymbolData engine::editor::ScriptEditorProvider::GetHoveredSymbol(kui::Vec2f ScreenPosition)
{
	if (Context->ScriptServiceMutex.try_lock())
	{
		auto HoverPosition = ParentEditor->ScreenToEditor(ScreenPosition, false);

		if (Context->ScriptService->files.empty())
		{
			Context->ScriptServiceMutex.unlock();
			return HoverSymbolData();
		}

		auto sym = GetSymbolAt(HoverPosition);
		Context->ScriptServiceMutex.unlock();
		return sym;
	}
	return HoverSymbolData();
}

ScriptEditorProvider::HoverSymbolData engine::editor::ScriptEditorProvider::GetSymbolAt(kui::EditorPosition Position)
{
	for (auto& i : Context->ScriptService->files[EditedFile].functions)
	{
		if (Position.Line == i.at.position.line
			&& (Position.Column >= i.at.position.startPos && Position.Column <= i.at.position.endPos))
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
					"\nDefined in " + Fn->definition->file
					+ " at line " + std::to_string(Fn->definition->at.position.line + 1)
					: "\nDefined in native code.", this->TextColor));

				return (new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 1000_px)
					->SetPadding(5_px);
			};

			std::function<SymbolDefinition()> GetDefinition;

			if (i.definition)
			{
				GetDefinition = [i] {
					return SymbolDefinition{ i.definition->at, i.definition->file };
				};
			}

			return HoverSymbolData{
				.Symbol = &i,
				.At = Position,
				.GetHoverData = Callback,
				.GetDefinition = GetDefinition,
			};
		}
	}

	for (auto& i : Context->ScriptService->files[EditedFile].variables)
	{
		if (Position.Line == i.at.position.line
			&& (Position.Column >= i.at.position.startPos && Position.Column <= i.at.position.endPos))
		{
			auto Callback = [this, i] {
				auto DefaultColor = this->TextColor;

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

			std::function<SymbolDefinition()> GetDefinition;

			if (i.definition && (i.definition->at.position.line != 0 || i.definition->at.position.endPos != 0))
			{
				GetDefinition = [i] {
					return SymbolDefinition{ i.definition->at, i.definition->file };
				};
			}

			return HoverSymbolData{
				.Symbol = &i,
				.At = Position,
				.GetHoverData = Callback,
				.GetDefinition = GetDefinition,
			};
		}
	}

	return HoverSymbolData();
}

void engine::editor::ScriptEditorProvider::UpdateLineColorization(size_t Line)
{
	if (!ParentEditor->IsLineLoaded(Line))
	{
		return;
	}

	auto found = Highlights.find(Line);

	if (found == Highlights.end())
	{
		std::vector<TextSegment> Segments;
		EngineTextEditorProvider::GetLine(Line, Segments);
		UpdateLine(Line, Segments);
		return;
	}

	std::sort(found->second.begin(), found->second.end(), []
	(const ScriptSyntaxHighlight& a, const ScriptSyntaxHighlight& b) {
		return a.Start < b.Start;
	});

	size_t LastStart = 0;
	std::vector<EditorColorizeSegment> Segments;

	for (ScriptSyntaxHighlight& i : found->second)
	{
		if (i.Start <= LastStart)
		{
			continue;
		}

		Segments.push_back(EditorColorizeSegment{
			.Offset = i.Start - LastStart,
			.Length = i.Length,
			.Color = i.Color,
			});
		LastStart = i.Start + i.Length;
	}

	ParentEditor->ColorizeLineLocking(Line, Segments);
}

void engine::editor::ScriptEditorProvider::UpdateFileContent()
{
	Context->UpdateFile(this->GetContent(), this->EditedFile);
	ScanFile();
}

void engine::editor::ScriptEditorProvider::UpdateFileData()
{
	UpdateSyntaxHighlight();

	for (size_t i = 0; i < Lines.size(); i++)
	{
		UpdateLineColorization(i);
	}
	ParentEditor->RefreshHighlights();
	//OnUpdated.Invoke();
}

void engine::editor::ScriptEditorProvider::ScanFile()
{
	Context->Commit([this] {
		UpdateFileData();
	});
}

void engine::editor::ScriptEditorProvider::UpdateSyntaxHighlight()
{
	Highlights.clear();
	std::lock_guard g{ Context->ScriptServiceMutex };

	auto& f = Context->ScriptService->files[EditedFile];
	for (auto& i : f.functions)
	{
		if (i.kind == ScannedFunction::Kind::constructor)
		{
			continue;
		}

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

	for (auto& i : f.types)
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

	for (auto& i : f.variables)
	{
		Highlights[i.at.position.line].push_back(ScriptSyntaxHighlight{
			.Start = i.at.position.startPos,
			.Length = i.at.position.endPos - i.at.position.startPos,
			.Color = i.isThis ? KeywordColor : VariableColor,
			});
	}
}

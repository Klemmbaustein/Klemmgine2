#include "ScriptEditorProvider.h"
#include <kui/UI/UITextEditor.h>
#include <ds/service/languageService.hpp>
#include <Editor/UI/DropdownMenu.h>
#include <algorithm>
#include <kui/Window.h>
#include <Core/Log.h>
#include <Editor/UI/EditorUI.h>

using namespace kui;
using namespace ds;
using namespace engine::editor;

engine::editor::ScriptEditorProvider::ScriptEditorProvider(std::string ScriptFile, ScriptEditorContext* Context,
	thread::ThreadMessagesRef Queue)
	: EngineTextEditorProvider(ScriptFile)
{
	this->Queue = Queue;
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
	Context->RemoveFile(this->EditedFile);
	Context->Commit(nullptr, nullptr);
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

void engine::editor::ScriptEditorProvider::GetLine(size_t LineIndex, std::vector<TextSegment>& To)
{
	FileEditorProvider::GetLine(LineIndex, To);
	UpdateLineColorization(LineIndex);
}

void engine::editor::ScriptEditorProvider::OnLoaded()
{
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

void engine::editor::ScriptEditorProvider::OnRightClick()
{
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

	if (GetIsAutoCompleteActive() || !DropdownMenu::Current.empty() || !Win->UI.HoveredBox
		|| !Win->UI.HoveredBox->IsChildOf(ParentEditor))
	{
		NewHovered = nullptr;
	}

	float Time = HoverTime.Get();

	UpdateAutoComplete();

	if (NewHovered != this->HoveredData || (Time > 0.2f && !GetHoverBox() && HoveredData))
	{
		if (NewHovered != this->HoveredData && !GetIsAutoCompleteActive())
		{
			this->HoveredData = NewHovered;
			Time = 0;
			HoverTime.Reset();
			ClearHovered();
		}

		if (NewHovered && Time > 0.2f)
		{
			if (NewHoveredError.Error)
			{
				CreateHoverBox((new UIText(12_px, EditorUI::Theme.Text, NewHoveredError.Error->Description, EditorUI::MonospaceFont))
					->SetPadding(5_px), NewHoveredError.At);
			}
			else
			{
				CreateHoverBox(NewHoveredSymbol.GetHoverData(), NewHoveredSymbol.At);
			}
		}
		else if (!NewHovered && !GetIsAutoCompleteActive())
		{
			HoverTime.Reset();
			ClearHovered();
		}
	}

	if (!NewHovered)
	{
		HoverTime.Reset();
	}

	LastCursorPosition = Win->Input.MousePosition;
}

std::vector<ds::AutoCompleteResult> engine::editor::ScriptEditorProvider::GetCompletionsAt(
	kui::EditorPosition At, CompletionSource Source)
{
	return Context->CompleteAt(EditedFile, At.Column, At.Line,
		Source == CompletionSource::TriggerChar ? CompletionType::classMembers : CompletionType::all,
		CompletionUsingLine);
}

void engine::editor::ScriptEditorProvider::NavigateTo(kui::EditorPosition Position)
{
	NavigateTo(Position, Position);
}

void engine::editor::ScriptEditorProvider::NavigateTo(kui::EditorPosition StartPosition,
	kui::EditorPosition EndPosition)
{
	ParentEditor->Edit();
	ParentEditor->SetCursorPosition(StartPosition,
		EditorPosition(EndPosition));
	ParentEditor->ScrollTo(ParentEditor->SelectionStart);
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

std::optional<ScriptEditorProvider::HoverSymbolData> engine::editor::ScriptEditorProvider::GetHoveredFunction(
	kui::EditorPosition Position, ds::ScannedFile& File)
{
	for (auto& i : File.functions)
	{
		if (Position.Line != i.at.position.line
			|| (Position.Column < i.at.position.startPos || Position.Column > i.at.position.endPos))
		{
			continue;
		}
		auto Callback = [this, Fn = &i] {
			std::vector<TextSegment> HoverString = {
				TextSegment(Fn->isVirtual ? "virtual fn " : "fn ", this->KeywordColor),
				TextSegment(Fn->shortName, this->FunctionColor),
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

			string InfoText;

			auto ResultUI = (new UIBox(false))
				->AddChild((new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(5_px));

			auto Documentation = EditorUI::Instance->Documentation.GetFunction(Fn->name);

			if (Documentation)
			{
				InfoText.append(Documentation->Description + "\n\n");

				if (!Documentation->Arguments.empty())
				{
					InfoText.append("Arguments:\n");
					for (auto& i : Documentation->Arguments)
					{
						InfoText.append("\t" + i.Name + ": " + i.Description + "\n");
					}
					InfoText.append("\n");
				}

				if (!Documentation->ReturnValueDescription.empty())
				{
					InfoText.append("Returns: " + Documentation->ReturnValueDescription + "\n\n");
				}
			}

			InfoText.append(Fn->definition ?
				"Defined in " + Fn->definition->file
				+ " at line " + std::to_string(Fn->definition->at.position.line + 1)
				: "Defined in native code");


			auto FoundColon = Fn->name.find_last_of(':');

			string Module = FoundColon != string::npos ? Fn->name.substr(0, FoundColon - 1) : "";

			if (Module.empty())
			{
				InfoText.append(", in the global module");
			}
			else
			{
				InfoText.append(", in module " + Module);
			}

			ResultUI
				->AddChild((new UIText(12_px, EditorUI::Theme.Text, InfoText, EditorUI::EditorFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(0, 5_px, 5_px, 15_px));

			return ResultUI;
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
	return std::nullopt;
}

std::optional<ScriptEditorProvider::HoverSymbolData> engine::editor::ScriptEditorProvider::GetHoveredVariable(
	kui::EditorPosition Position, ds::ScannedFile& File)
{
	for (auto& i : File.variables)
	{
		if (Position.Line != i.at.position.line
			|| (Position.Column < i.at.position.startPos || Position.Column > i.at.position.endPos))
		{
			continue;
		}

		auto Callback = [this, i] {
			auto DefaultColor = this->TextColor;

			string Description;

			static std::map<ScannedVariable::Kind, string> Type = {
				{ScannedVariable::Kind::localVariable, "Local variable"},
				{ScannedVariable::Kind::classMember, "Class member"},
				{ScannedVariable::Kind::constant, "Constant"},
			};

			std::vector<TextSegment> HoverString = {
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

			Description.append(Type[i.kind]);

			if (i.kind == ScannedVariable::Kind::classMember)
			{
				auto FoundMember = EditorUI::Instance->Documentation.GetTypeMember(i.inClass, i.name);
				if (FoundMember)
				{
					Description.append("\n" + FoundMember->Description);
				}
			}

			return (new UIBox(false))
				->AddChild((new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(5_px))
				->AddChild((new UIText(12_px, EditorUI::Theme.Text, Description, EditorUI::EditorFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(0, 5_px, 5_px, 15_px));
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

	return std::nullopt;
}

std::optional<ScriptEditorProvider::HoverSymbolData> engine::editor::ScriptEditorProvider::GetHoveredType(
	kui::EditorPosition Position, ds::ScannedFile& File)
{
	for (auto& i : File.types)
	{
		if (Position.Line != i.at.position.line
			|| (Position.Column < i.at.position.startPos || Position.Column > i.at.position.endPos))
		{
			continue;
		}

		auto BaseType = this->Context->ScriptService->types.find(i.id);
		ScannedType* FoundType = nullptr;
		std::function<SymbolDefinition()> GetDefinition;

		if (BaseType != this->Context->ScriptService->types.end())
		{
			auto& t = BaseType->second;
			FoundType = &t;

			if (t.definition && (t.definition->at.position.line != 0 || t.definition->at.position.endPos != 0))
			{
				GetDefinition = [t] {
					return SymbolDefinition{ t.definition->at, t.definition->file };
				};
			}
		}

		auto Callback = [this, i, FoundType] {
			auto DefaultColor = this->TextColor;

			string Description;

			std::vector<TextSegment> HoverString = {};

			if (i.isInterface)
			{
				HoverString.push_back(TextSegment("interface ", this->KeywordColor));
			}
			else if (i.isClass)
			{
				HoverString.push_back(TextSegment("class ", this->KeywordColor));
			}
			else if (i.isAttribute)
			{
				HoverString.push_back(TextSegment("attribute ", this->KeywordColor));
			}
			else if (!i.isPrimitive)
			{
				HoverString.push_back(TextSegment("struct ", this->KeywordColor));
			}

			HoverString.push_back(TextSegment(i.at.string, this->TypeColor));

			auto box = (new UIBox(false))
				->AddChild((new UIText(12_px, HoverString, EditorUI::MonospaceFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(5_px));

			if (FoundType)
			{
				auto FoundDocumentation = EditorUI::Instance->Documentation.GetType(i.fullName);
				if (FoundDocumentation)
				{
					Description.append(FoundDocumentation->Description + "\n\n");
				}

				Description.append(FoundType->definition ?
					"Defined in " + FoundType->definition->file
					+ " at line " + std::to_string(FoundType->definition->at.position.line + 1)
					: "Defined in native code");

				if (i.module.empty())
				{
					Description.append(", in the global module");
				}
				else
				{
					Description.append(", in module " + i.module);
				}

				box->AddChild((new UIText(12_px, EditorUI::Theme.Text, Description, EditorUI::EditorFont))
					->SetWrapEnabled(true, 800_px)
					->SetPadding(0, 5_px, 5_px, 15_px));
			}

			return box;
		};

		return HoverSymbolData{
			.Symbol = &i,
			.At = Position,
			.GetHoverData = Callback,
			.GetDefinition = GetDefinition,
		};
	}
	return std::nullopt;
}

ScriptEditorProvider::HoverSymbolData engine::editor::ScriptEditorProvider::GetSymbolAt(kui::EditorPosition Position)
{
	auto& File = Context->ScriptService->files[EditedFile];

	auto HoveredFunction = GetHoveredFunction(Position, File);
	if (HoveredFunction)
	{
		return *HoveredFunction;
	}

	auto HoveredVariable = GetHoveredVariable(Position, File);
	if (HoveredVariable)
	{
		return *HoveredVariable;
	}

	auto HoveredType = GetHoveredType(Position, File);
	if (HoveredType)
	{
		return *HoveredType;
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
}

void engine::editor::ScriptEditorProvider::ScanFile()
{
	Context->Commit([this] {
		UpdateFileData();
	}, Queue);
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
		size_t ActualStart = i.at.position.startPos;

		size_t Colon = i.at.string.find_last_of(':');

		if (Colon != std::string::npos)
		{
			ActualStart += Colon + 1;
		}

		Highlights[i.at.position.line].push_back(ScriptSyntaxHighlight{
			.Start = ActualStart,
			.Length = i.at.position.endPos - ActualStart,
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

#include "SaveLayout.h"
#include <Core/File/BinarySerializer.h>
#include <Editor/UI/Panels/AssetBrowser.h>
#include <Editor/UI/Panels/ClassBrowser.h>
#include <Editor/UI/Panels/Viewport.h>
#include <Editor/UI/Panels/ScriptEditorPanel.h>
#include <Editor/UI/Panels/ObjectListPanel.h>
#include <Editor/UI/Panels/ConsolePanel.h>
#include <Editor/UI/Panels/PropertyPanel.h>
#include <Editor/UI/Panels/ScenePanel.h>
#include <Editor/UI/Panels/MessagePanel.h>

using namespace engine;

void engine::editor::layout::LayoutToFile(EditorPanel* Root, string File)
{
	std::vector<SerializedData> Result;

	Result.push_back(SerializedData("root", SerializePanel(Root)));

	BinarySerializer::ToFile(Result, File);
}

// TODO: Verify there is at least but only ever one viewport in the layout
// since the editor expects this to always be true for the layout.
void engine::editor::layout::LoadLayout(EditorPanel* Root, string File)
{
	SerializedValue FileContent = BinarySerializer::FromFile(File);

	if (!FileContent.Contains("root"))
	{
		return;
	}

	DeSerializePanel(Root, FileContent.At("root"));
}

SerializedValue engine::editor::layout::SerializePanel(EditorPanel* Target)
{
	std::vector<SerializedData> Result;

	Result.push_back({ "name", Target->GetTypeName() });
	Result.push_back({ "size", Target->SizeFraction });
	Result.push_back({ "align", int32(Target->ChildrenAlign) });
	Result.push_back({ "data", Target->Serialize() });

	std::vector<SerializedValue> Children;

	for (EditorPanel* p : Target->Children)
	{
		Children.push_back(SerializePanel(p));
	}

	Result.push_back({ "children", Children });

	return Result;
}

void engine::editor::layout::DeSerializePanel(EditorPanel* Target, SerializedValue& Obj)
{
	if (Target->Parent)
	{
		if (Obj.Contains("data"))
		{
			Target->DeSerialize(&Obj.At("data"));
		}
		Target->SizeFraction = Obj.At("size").GetFloat();
	}

	auto Align = EditorPanel::Align(Obj.At("align").GetInt());

	auto& Children = Obj.At("children");
	for (auto& i : Children.GetArray())
	{
		EditorPanel* NewPanel = nullptr;
		bool IsDefaultPanel = false;

		string Type = i.At("name").GetString();

		if (Type == "asset_browser")
		{
			NewPanel = new AssetBrowser();
		}
		else if (Type == "class_browser")
		{
			NewPanel = new ClassBrowser();
		}
		else if (Type == "console")
		{
			NewPanel = new ConsolePanel();
		}
		else if (Type == "viewport")
		{
			NewPanel = new Viewport();
		}
		else if (Type == "scripts")
		{
			NewPanel = new ScriptEditorPanel();
		}
		else if (Type == "object_list")
		{
			NewPanel = new ObjectListPanel();
		}
		else if (Type == "object_properties")
		{
			NewPanel = new PropertyPanel();
		}
		else if (Type == "scene")
		{
			NewPanel = new ScenePanel();
		}
		else if (Type == "messages")
		{
			NewPanel = new MessagePanel();
		}
		else if (Type == "panel")
		{
			IsDefaultPanel = true;
			NewPanel = new EditorPanel(Type, Type);
		}
		else
		{
			continue;
		}
		Target->AddChild(NewPanel, Align);

		DeSerializePanel(NewPanel, i);

		if (IsDefaultPanel && NewPanel->Children.empty())
		{
			delete NewPanel;
		}
	}
}

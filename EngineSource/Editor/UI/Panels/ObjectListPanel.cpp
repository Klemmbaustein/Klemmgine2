#ifdef EDITOR
#include "ObjectListPanel.h"
#include <Engine/Scene.h>
#include <Core/File/FileUtil.h>
#include "Viewport.h"
#include <iostream>
#include <Engine/Input.h>
#include <Editor/UI/EditorUI.h>
using namespace kui;

engine::editor::ObjectListPanel::ObjectListPanel()
	: EditorPanel("Objects", "object_list")
{
	Heading = new ObjectListHeader();
	Background->AddChild(Heading);

	DisplayList();
}

void engine::editor::ObjectListPanel::Update()
{
	Heading->SetMinSize(Size - UIBox::PixelSizeToScreenSize(2, Heading->GetParentWindow()));
	Heading->search->SetSize(UISize::Pixels(Heading->GetMinSize().GetPixels().X - 30));

	Heading->listBox->SetMinSize(Size - UIBox::PixelSizeToScreenSize(Vec2f(2, 35), Heading->GetParentWindow()));
	Heading->listBox->SetMaxSize(Heading->listBox->GetMinSize());

	Scene* Current = Scene::GetMain();

	if ((EditorUI::FocusedPanel == this || EditorUI::FocusedPanel == Viewport::Current)
		&& input::IsKeyDown(input::Key::ESCAPE)
		&& !Viewport::Current->SelectedObjects.empty())
	{
		Viewport::Current->SelectedObjects.clear();
		DisplayList();
	}

	if (!Current && LastLength != SIZE_MAX)
	{
		LastLength = SIZE_MAX;
		LastSelectedLength = 0;
		DisplayList();
	}
	else if (Current && LastLength != Current->Objects.size())
	{
		LastLength = Current->Objects.size();
		LastSelectedLength = Viewport::Current->SelectedObjects.size();
		DisplayList();
	}

	if (LastSelectedLength != Viewport::Current->SelectedObjects.size())
	{
		LastSelectedLength = Viewport::Current->SelectedObjects.size();
		LastSelectedObj = LastSelectedLength  ? *Viewport::Current->SelectedObjects.begin() : nullptr;
		DisplayList();
	}
	else if (LastSelectedLength && *Viewport::Current->SelectedObjects.begin() != LastSelectedObj)
	{
		LastSelectedObj = *Viewport::Current->SelectedObjects.begin();
		DisplayList();
	}
}

void engine::editor::ObjectListPanel::DisplayList()
{
	Heading->listBox->DeleteChildren();

	Scene* Current = Scene::GetMain();

	if (!Current)
		return;

	auto& Objects = Scene::GetMain()->Objects;

	string Name = file::FileNameWithoutExt(Current->Name) + " (Scene)";

	std::map<string, ListObject> ObjectTree;

	auto& SceneMap = ObjectTree.insert(
		{ Name, ListObject{.Name = Name} }
	).first->second.Children;


	size_t Number = 0;
	for (SceneObject* i : Objects)
	{
		if (Current->ObjectDestroyed(i))
			continue;

		const Reflection::ObjectInfo& TypeInfo = Reflection::ObjectTypes[i->TypeID];

		auto& Entry = SceneMap[TypeInfo.Path];

		Entry.Children.insert(
			{ std::to_string(Number++), ListObject(i->Name, i, Viewport::Current->SelectedObjects.contains(i))}
		);
		if (Entry.Name.empty())
		{
			Entry.Name = TypeInfo.Path;
		}
	}

	AddListObjects(ObjectTree, 0);
	Heading->UpdateElement();
	Heading->RedrawElement();
}

void engine::editor::ObjectListPanel::AddListObjects(const std::map<string, ListObject>& Objects, size_t Depth)
{
	for (auto& [_, Obj] : Objects)
	{
		auto* Elem = new ObjectEntryElement();

		Elem->SetName(Obj.Name);
		Elem->SetLeftPadding(UISize::Pixels(16 * Depth + (Obj.Children.empty() ? 21 : 5)));
		Elem->SetColor(Obj.Selected
			? EditorUI::Theme.Highlight1
			: (Heading->listBox->GetChildren().size() % 2 == 0
				? EditorUI::Theme.LightBackground : EditorUI::Theme.Background));

		Elem->SetTextColor(Obj.Selected ? EditorUI::Theme.HighlightText : EditorUI::Theme.Text);
		Elem->SetIcon(!Obj.From ? "Engine/Editor/Assets/Folder.png" : "");

		bool HasChildren = Obj.Children.size();
		bool IsCollapsed = false;

		if (HasChildren)
		{
			IsCollapsed = Collapsed.contains(Obj.Name);
			Elem->SetArrowImage(IsCollapsed ? "Engine/Editor/Assets/LeftArrow.png" : "Engine/Editor/Assets/DownArrow.png");
		}

		if (Obj.Children.empty())
			delete Elem->arrow;

		Elem->button->OnClicked = [this, Obj]()
			{
				if (!input::IsKeyDown(input::Key::LSHIFT))
					Viewport::Current->SelectedObjects.clear();

				if (Obj.From)
					Viewport::Current->SelectedObjects.insert(Obj.From);
				else
				{
					if (Collapsed.contains(Obj.Name))
					{
						Collapsed.erase(Obj.Name);
					}
					else
					{
						Collapsed.insert(Obj.Name);
					}
				}
				DisplayList();
			};
		Heading->listBox->AddChild(Elem);

		if (HasChildren && !IsCollapsed)
		{
			AddListObjects(Obj.Children, Depth + 1);
		}
	}
}
#endif
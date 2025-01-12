#ifdef EDITOR
#include "ObjectListPanel.h"
#include <Engine/Scene.h>
#include <Engine/File/FileUtil.h>
#include "Viewport.h"
#include <iostream>
#include <Engine/Input.h>
#include <Engine/Editor/UI/EditorUI.h>
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

	if (EditorUI::FocusedPanel == this
		&& input::IsKeyDown(input::Key::ESCAPE)
		&& !Viewport::Current->SelectedObjects.empty())
	{
		Viewport::Current->SelectedObjects.clear();
		DisplayList();
	}
	if (EditorUI::FocusedPanel == this
		&& input::IsKeyDown(input::Key::DELETE)
		&& !Viewport::Current->SelectedObjects.empty())
	{
		Viewport::Current->RemoveSelected();
		DisplayList();
	}

	if (!Current && LastLength != SIZE_MAX)
	{
		LastLength = SIZE_MAX;
		DisplayList();
	}
	else if (Current && LastLength != Current->Objects.size())
	{
		LastLength = Current->Objects.size();
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

	std::vector<ListObject> ObjectTree = {
		ListObject{.Name = file::FileNameWithoutExt(Current->Name) + " (Scene)"}
	};

	for (SceneObject* i : Objects)
	{
		if (Current->ObjectDestroyed(i))
			continue;
		ObjectTree[0].Children.emplace_back(i->Name, i, Viewport::Current->SelectedObjects.contains(i));
	}

	AddListObjects(ObjectTree, 0);
	Heading->UpdateElement();
	Heading->RedrawElement();
}

void engine::editor::ObjectListPanel::AddListObjects(const std::vector<ListObject>& Objects, size_t Depth)
{
	for (auto& Obj : Objects)
	{
		auto* Elem = new ObjectEntryElement();

		Elem->SetName(Obj.Name);
		Elem->SetLeftPadding(UISize::Pixels(16 * Depth + (Obj.Children.empty() ? 21 : 5)));
		Elem->SetColor(Obj.Selected
			? EditorUI::Theme.Highlight1
			: (Heading->listBox->GetChildren().size() % 2 == 0
			? EditorUI::Theme.LightBackground : EditorUI::Theme.Background));
		Elem->SetTextColor(Obj.Selected ? EditorUI::Theme.HighlightText : EditorUI::Theme.Text);
		if (Obj.Children.empty())
			delete Elem->arrow;
		Elem->button->OnClicked = [this, Obj]()
		{
			if (!input::IsKeyDown(input::Key::LSHIFT))
				Viewport::Current->SelectedObjects.clear();

			if (Obj.From)
				Viewport::Current->SelectedObjects.insert(Obj.From);
			DisplayList();
		};
		Heading->listBox->AddChild(Elem);

		if (!Obj.Children.empty())
		{
			AddListObjects(Obj.Children, Depth + 1);
		}
	}
}
#endif
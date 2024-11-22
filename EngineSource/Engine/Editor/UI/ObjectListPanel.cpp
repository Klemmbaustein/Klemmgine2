#include "ObjectListPanel.h"
#include <Engine/Scene.h>
#include <Engine/File/FileUtil.h>
#include <iostream>
#include "EditorUI.h"

using namespace kui;

engine::editor::ObjectListPanel::ObjectListPanel()
	: EditorPanel("Objects", "object_list")
{
	Heading = new ObjectListHeader();
	Heading->search->field->SetSizeMode(UIBox::SizeMode::ScreenRelative);
	Background->AddChild(Heading);

	DisplayList();
}

void engine::editor::ObjectListPanel::Update()
{
	Heading->SetMinSize(Size - UIBox::PixelSizeToScreenSize(2, Heading->GetParentWindow()));
	Heading->search->field->SetMinSize(
		Vec2f(Size.X, 0) + UIBox::PixelSizeToScreenSize(Vec2f(-32, 22), Heading->GetParentWindow())
	);

	Scene* Current = Scene::GetMain();

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
		ListObject{
			.Name = file::FileNameWithoutExt(Current->Name) + " (Scene)"
		}
	};

	for (auto& i : Objects)
	{
		ObjectTree[0].Children.emplace_back(i->Name, i);
	}

	AddListObjects(ObjectTree, 0);
	Heading->RedrawElement();
}

void engine::editor::ObjectListPanel::AddListObjects(const std::vector<ListObject>& Objects, size_t Depth)
{
	for (auto& Obj : Objects)
	{
		auto* Elem = new ObjectEntryElement();

		Elem->SetName(Obj.Name);
		Elem->SetLeftPadding(float(16 * Depth + (Obj.Children.empty() ? 21 : 5)));
		Elem->SetColor(Heading->listBox->GetChildren().size() % 2 == 0 
			? EditorUI::Theme.LightBackground : EditorUI::Theme.Background);
		if (Obj.Children.empty())
			delete Elem->arrow;
		Heading->listBox->AddChild(Elem);

		if (!Obj.Children.empty())
		{
			AddListObjects(Obj.Children, Depth + 1);
		}
	}
}
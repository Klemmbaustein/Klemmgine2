#ifdef EDITOR
#include "DroppableBox.h"
#include <Engine/Log.h>
using namespace engine::editor;

thread_local std::vector<DroppableBox*> DroppableBox::CurrentBoxes;

engine::editor::DroppableBox::DroppableBox(bool IsHorizontal, OnDropFn OnDrop)
	: UIBox(IsHorizontal)
{
	this->OnDrop = OnDrop;
	CurrentBoxes.push_back(this);
}
engine::editor::DroppableBox::~DroppableBox()
{
	for (auto i = CurrentBoxes.begin(); i < CurrentBoxes.end(); i++)
	{
		if (*i == this)
		{
			CurrentBoxes.erase(i);
			break;
		}
	}
}
DroppableBox* engine::editor::DroppableBox::GetBoxAtCursor()
{
	DroppableBox* Found = nullptr;
	for (DroppableBox* i : CurrentBoxes)
	{
		if (i->IsBeingHovered())
		{
			Found = i;
		}
	}
	return Found;
}
#endif
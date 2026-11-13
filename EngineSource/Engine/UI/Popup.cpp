#include "Popup.h"
#include <kui/Window.h>
#include <Engine/Input.h>

using namespace kui;
using namespace engine;

engine::Popup::Popup(bool Clear)
{
	if (Clear)
	{
		Popup::Clear();
	}

	Current.push_back(this);
}

thread_local std::vector<Popup*> Popup::Current;

engine::Popup::~Popup()
{
	if (Window::GetActiveWindow()->Input.KeyboardFocusTargetBox == Box)
	{
		Window::GetActiveWindow()->Input.KeyboardFocusTargetBox = nullptr;
	}
	if (Box)
	{
		Box->RemoveDeletedListener(this);
		delete Box;
	}
}

void engine::Popup::SetBox(kui::UIBox* NewBox)
{
	NewBox->AddDeletedListener(this, [this] {
		this->Box = nullptr;
		Popup::Clear();
	});
	Box = NewBox;
}

void engine::Popup::Clear(size_t ToDepth)
{
	while (Current.size() > ToDepth)
	{
		delete* Current.rbegin();
		Current.pop_back();
	}
}

void engine::Popup::UpdatePopups()
{
	bool IsHovered = false;

	for (auto& i : Current)
	{
		if (i->Box->IsBeingHovered())
		{
			IsHovered = true;
		}
	}

	if (!Current.empty()
		&& (((input::IsLMBClicked || input::IsRMBClicked) && !IsHovered)
			|| input::IsKeyHeld(input::Key::ESCAPE) || !input::ShowMouseCursor))
	{
		Clear();
	}
}


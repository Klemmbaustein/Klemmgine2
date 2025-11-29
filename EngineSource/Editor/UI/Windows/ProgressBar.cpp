#include "ProgressBar.h"
#include <cmath>
#include <algorithm>

engine::editor::ProgressBar::ProgressBar(string Title)
	: IPopupWindow(Title, kui::Vec2ui(350, 70), false, false)
{
	Open();
}

void engine::editor::ProgressBar::Begin()
{
	Elem = new LoadingBarElement();
}

void engine::editor::ProgressBar::Update()
{
	{
		std::lock_guard g{ ProgressMessageMutex };

		if (UpdateTitle)
		{
			Elem->SetLoadingText(ProgressMessage);
		}
	}

	float Size = Elem->bar->GetUsedSize().GetScreen().X;
	if (Progress < 0)
	{
		Time += Popup->GetDeltaTime();


		float ProgressValue = std::fmod(Time, 1.5f);

		Elem->SetBarLeftPadding(kui::UISize::Screen(Size * std::clamp(ProgressValue - 0.3f, 0.0f, 1.0f)));
		Elem->SetBarRightPadding(kui::UISize::Screen(Size * std::clamp(1 - ProgressValue, 0.0f, 1.0f)));
	}
	else
	{
		Elem->SetBarLeftPadding(0_px);
		Elem->SetBarRightPadding(kui::UISize::Screen(Size * (1 - Progress)));
	}
}

void engine::editor::ProgressBar::Destroy()
{
}

void engine::editor::ProgressBar::SetMessage(string NewMessage)
{
	std::lock_guard g{ ProgressMessageMutex };
	if (NewMessage != this->ProgressMessage)
	{
		UpdateTitle = true;
		this->ProgressMessage = NewMessage;
	}
}

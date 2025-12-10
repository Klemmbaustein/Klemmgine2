#pragma once
#include <kui/UI/UIButton.h>
#include <functional>

namespace engine::editor
{
	class UICheckbox : public kui::UIButton
	{
	public:

		UICheckbox(bool Value, std::function<void()> OnClicked);

		void OnButtonClicked() override;

		bool Value = false;
	};
}

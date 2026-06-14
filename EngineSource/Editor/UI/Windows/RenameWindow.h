#pragma once
#include "IDialogWindow.h"
#include <kui/UI/UITextField.h>

namespace engine::editor
{
	class RenameWindow : public IDialogWindow
	{
	public:
		RenameWindow(string File, std::function<void(string NewName)> OnRenamed, bool IsNewFile = false);

		void Begin() override;
		void Update() override;
		void Destroy() override;
		void Confirm();

	private:
		kui::UITextField* EditField = nullptr;
		string File;
		bool IsNewFile = false;
		std::function<void(string NewName)> OnRenamed;
	};
}

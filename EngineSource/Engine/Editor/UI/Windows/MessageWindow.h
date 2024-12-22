#pragma once
#include "IDialogWindow.h"

namespace engine::editor
{
	class MessageWindow : public IDialogWindow
	{
	public:
		MessageWindow(string Message, std::function<void()> OnAccepted);
		MessageWindow(string Message, string Title, std::function<void()> OnAccepted);
		MessageWindow(string Message, string Title, std::vector<Option> Options);

		void Begin() override;
		void Update() override;
		void Destroy() override;
	private:
		string Message;
	};
}
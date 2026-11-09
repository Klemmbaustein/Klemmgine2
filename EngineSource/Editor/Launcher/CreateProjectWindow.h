#pragma once
#include <Editor/UI/Windows/DialogWindow.h>
#include <Editor/UI/Elements/PropertyMenu.h>

namespace engine::editor::launcher
{
	class CreateProjectWindow : public DialogWindow
	{
	public:
		CreateProjectWindow(std::function<void(std::string Path)> OnAccept);

		void Begin() override;

		// Inherited via DialogWindow
		void Update() override;

		void Destroy() override;

	private:
		PropertyMenu* Menu = nullptr;
		string Path;
		string ProjectName;

		kui::UIText* ResultText = nullptr;

		std::function<void(std::string Path)> OnAccept;

		void Accept();
	};
}
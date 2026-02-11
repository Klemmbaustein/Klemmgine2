#pragma once
#include <Editor/UI/Windows/IDialogWindow.h>
#include <Editor/UI/Elements/PropertyMenu.h>

namespace engine::editor::launcher
{
	class CreateProjectWindow : public IDialogWindow
	{
	public:
		CreateProjectWindow(std::function<void(std::string Path)> OnAccept);

		void Begin() override;

		// Inherited via IDialogWindow
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
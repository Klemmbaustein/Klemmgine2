#pragma once
#include <Editor/UI/Windows/IDialogWindow.h>
#include <NewProjectWindow.kui.hpp>

namespace engine::editor
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

		NewProjectWindowElement* Element = nullptr;

		std::function<void(std::string Path)> OnAccept;

		void Accept();
	};
}
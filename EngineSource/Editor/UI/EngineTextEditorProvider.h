#pragma once
#include <kui/UI/FileEditorProvider.h>
#include <kui/UI/TextEditor.h>
#include <Core/Types.h>
#include <stack>
#include <Editor/UI/DropdownMenu.h>

namespace engine::editor
{
	/**
	 * @brief
	 * A text editor provider that contains common engine functionality (right click menu)
	 */
	class EngineTextEditorProvider : public kui::FileEditorProvider
	{
	public:

		EngineTextEditorProvider(std::string File);
		virtual ~EngineTextEditorProvider() override;

		string EditedFile;

		virtual void Update();

		virtual std::vector<DropdownMenu::Option> GetRightClickOptions(kui::EditorPosition At);

		virtual void OnRightClick();
	};
}
#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/ProjectFile.h>

namespace engine::editor
{
	class EditorSubsystem : public subsystem::Subsystem
	{
	public:
		EditorSubsystem();
		virtual ~EditorSubsystem() override;

		virtual void Update() override;

		void StartProject();
		void StopProject();
		void RegisterCommands(ConsoleSubsystem* System) override;

		EditorUI* UI = nullptr;

		static bool Active;

		ProjectFile Project = ProjectFile("./project.json");
	};
}

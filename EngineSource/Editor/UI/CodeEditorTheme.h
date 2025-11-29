#pragma once
#include <kui/Vec3.h>
#include <Core/Types.h>
#include <Editor/UI/ScriptEditorProvider.h>

namespace engine::editor
{
	struct CodeEditorTheme
	{
		string Name;

		kui::Vec3f Text = 1;
		kui::Vec3f Type = kui::Vec3f(1.0f, 0.8f, 0.1f);
		kui::Vec3f Function = kui::Vec3f(0.27f, 0.88f, 0.65f);
		kui::Vec3f Variable = kui::Vec3f(0.71f, 0.96f, 0.95f);
		kui::Vec3f String = kui::Vec3f(0.5f, 1.0f, 0.2f);
		kui::Vec3f Number = kui::Vec3f(0.5f, 0.7f, 1.0f);
		kui::Vec3f Keyword = kui::Vec3f(1.0f, 0.2f, 0.5f);
		bool IsLight = false;

		void ApplyToScript(ScriptEditorProvider* Provider) const;
		void ApplyToFile(kui::FileEditorProvider* Provider) const;


		void LoadFromFile(string ThemeName);
	};
}

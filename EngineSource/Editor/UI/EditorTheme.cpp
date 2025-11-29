#include "EditorTheme.h"
#include <Core/File/TextSerializer.h>
#include <Core/Log.h>
#include <map>

using namespace kui;

void engine::editor::EditorTheme::LoadFromFile(string ThemeName)
{
	*this = EditorTheme();

	this->Name = ThemeName;

	try
	{
		SerializedValue ThemeData = TextSerializer::FromFile("Engine/Editor/Themes/" + ThemeName + ".k2t");

		std::map<string, kui::Vec3f&> Colors =
		{
			{"text", this->Text},
			{"darkText", this->DarkText},
			{"background", this->Background},
			{"darkBackground", this->DarkBackground},
			{"darkBackground2", this->DarkBackground2},
			{"lightBackground", this->LightBackground},
			{"backgroundHighlight", this->BackgroundHighlight},
			{"darkBackgroundHighlight", this->DarkBackgroundHighlight},
			{"highlight1", this->Highlight1},
			{"highlightDark", this->HighlightDark},
			{"highlight2", this->Highlight2},
			{"highlightText", this->HighlightText},
		};

		auto ColorData = ThemeData.At("colors");

		this->IsLight = ThemeData.Contains("isLight") && ThemeData.At("isLight").GetBool();
		this->CornerSize = ThemeData.Contains("cornerSize") ? UISize::Pixels(ThemeData.At("cornerSize").GetInt()) : 5_px;

		this->CodeTheme = CodeEditorTheme();
		if (ThemeData.Contains("editorTheme"))
		{
			this->CodeTheme.LoadFromFile(ThemeData.At("editorTheme").GetString());
		}

		for (auto& [Name, Value] : Colors)
		{
			if (ColorData.Contains(Name))
			{
				Vector3 ValueVector = ColorData.At(Name).GetVector3();
				Value = kui::Vec3f(ValueVector.X, ValueVector.Y, ValueVector.Z);
			}
		}
	}
	catch (SerializeReadException e)
	{
		Log::Warn(e.what());
	}
}

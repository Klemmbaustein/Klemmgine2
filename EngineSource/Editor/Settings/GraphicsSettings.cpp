#include "GraphicsSettings.h"
#include <Engine/Graphics/VideoSubsystem.h>

engine::editor::GraphicsSettings::GraphicsSettings()
	: SettingsCategory("graphics")
{
	ListenToSetting(this, "drawShadows", [](SerializedValue NewValue) {
		if (VideoSubsystem::Current)
		{
			VideoSubsystem::Current->DrawShadows = NewValue.GetBool();
		}
	});
	ListenToSetting(this, "drawAmbientOcclusion", [](SerializedValue NewValue) {
		if (VideoSubsystem::Current)
		{
			VideoSubsystem::Current->DrawAmbientOcclusion = NewValue.GetBool();
		}
	});

	if (VideoSubsystem::Current)
	{
		VideoSubsystem::Current->DrawShadows = GetSetting("drawShadows", true).GetBool();
		VideoSubsystem::Current->DrawAmbientOcclusion = GetSetting("drawAmbientOcclusion", true).GetBool();
	}
}

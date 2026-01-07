#include "GraphicsSettingsPage.h"
#include <GL/glew.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Graphics/OpenGL.h>
#include <Editor/Settings/EditorSettings.h>

engine::editor::GraphicsSettingsPage::GraphicsSettingsPage()
{
	this->Name = "Graphics";
	this->Shadows = Settings::GetInstance()->Graphics.GetSetting("drawShadows", true).GetBool();
}

engine::editor::GraphicsSettingsPage::~GraphicsSettingsPage()
{
}

void engine::editor::GraphicsSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	Target->CreateNewHeading("Graphics");
	Target->AddBooleanEntry("Shadows", Shadows, [this]() {
		Settings::GetInstance()->Graphics.SetSetting("drawShadows", this->Shadows);
		this->Shadows = !this->Shadows;
	});
	Target->AddBooleanEntry("Ambient Occlusion", AmbientOcclusion, [this]() {
		Settings::GetInstance()->Graphics.SetSetting("drawAmbientOcclusion", AmbientOcclusion);
		this->AmbientOcclusion = !this->AmbientOcclusion;
	});
	Target->AddBooleanEntry("V-Sync", VideoSubsystem::Current ? VideoSubsystem::Current->VSyncEnabled : this->Vsync, nullptr);
	Target->CreateNewHeading("Driver information");
	Target->AddInfoEntry("OpenGL", (const char*)glGetString(GL_VERSION));
	Target->AddInfoEntry("GLSL", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	Target->AddInfoEntry("Mode", openGL::GetGLVersion() == openGL::Version::GL330 ? "OpenGL 3.3+" : "OpenGL 4.3+");
}

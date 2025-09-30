#include "GraphicsSettingsPage.h"
#include <GL/glew.h>
#include <Engine/Graphics/OpenGL.h>

engine::editor::GraphicsSettingsPage::GraphicsSettingsPage()
{
	this->Name = "Graphics";
}

engine::editor::GraphicsSettingsPage::~GraphicsSettingsPage()
{
}

void engine::editor::GraphicsSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	Target->CreateNewHeading("Graphics");
	Target->AddBooleanEntry("Anti aliasing", AntiAliasing, nullptr);
	Target->CreateNewHeading("Driver information");
	Target->AddInfoEntry("OpenGL", (const char*)glGetString(GL_VERSION));
	Target->AddInfoEntry("GLSL", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	Target->AddInfoEntry("Mode", openGL::GetGLVersion() == openGL::Version::GL330 ? "OpenGL 3.3+" : "OpenGL 4.3+");
}

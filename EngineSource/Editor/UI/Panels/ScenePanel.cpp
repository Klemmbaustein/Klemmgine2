#include "ScenePanel.h"
#include <Editor/UI/Panels/Viewport.h>

engine::editor::ScenePanel::ScenePanel()
	: EditorPanel("Scene", "ScenePanel")
{
	this->Properties = new PropertyMenu();

	Background->AddChild(Properties);
}

void engine::editor::ScenePanel::Update()
{
	if (CurrentScene != Scene::GetMain())
	{
		LoadPropertiesFrom(Scene::GetMain());
	}
}

void engine::editor::ScenePanel::OnResized()
{
	LoadPropertiesFrom(Scene::GetMain());
}

void engine::editor::ScenePanel::LoadPropertiesFrom(Scene* Target)
{
	Properties->SetMinSize(this->Size);
	Properties->SetMaxSize(this->Size);
	CurrentScene = Target;
	Properties->Clear();

	if (!Target)
	{
		return;
	}

	auto OnChanged = [] {
		Viewport::Current->SceneChanged();
	};

	Properties->CreateNewHeading("Scene - " + Target->Name);

	Properties->CreateNewHeading("Lighting");
	Properties->AddVecEntry("Sun color", Target->Graphics.SceneEnvironment.SunColor, OnChanged, true);
	Properties->AddVecEntry("Sun rotation", *(Vector3*)&Target->Graphics.SceneEnvironment.SunRotation, OnChanged, false, true);
	Properties->AddFloatEntry("Sun intensity", Target->Graphics.SceneEnvironment.SunIntensity, OnChanged);
	Properties->AddVecEntry("Sky color", Target->Graphics.SceneEnvironment.SkyColor, OnChanged, true);
	Properties->AddVecEntry("Ground color", Target->Graphics.SceneEnvironment.GroundColor, OnChanged, true);
	Properties->AddFloatEntry("Ambient intensity", Target->Graphics.SceneEnvironment.AmbientIntensity, OnChanged);

	Properties->CreateNewHeading("Atmosphere");

	Properties->AddVecEntry("Fog Color", Target->Graphics.SceneEnvironment.FogColor, OnChanged, true);
	Properties->AddFloatEntry("Fog range", Target->Graphics.SceneEnvironment.FogRange, OnChanged);
	Properties->AddFloatEntry("Fog start", Target->Graphics.SceneEnvironment.FogStart, OnChanged);
}

void engine::editor::ScenePanel::OnThemeChanged()
{
	OnResized();
}

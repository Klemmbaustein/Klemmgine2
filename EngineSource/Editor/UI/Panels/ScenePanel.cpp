#include "ScenePanel.h"
#include <Editor/UI/Panels/Viewport.h>

engine::editor::ScenePanel::ScenePanel()
	: EditorPanel("Scene", "scene")
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
	Properties->AddVecEntry("Sun color", Target->SceneEnvironment.SunColor, OnChanged, true);
	Properties->AddVecEntry("Sun rotation", *(Vector3*)&Target->SceneEnvironment.SunRotation, OnChanged);
	Properties->AddFloatEntry("Sun intensity", Target->SceneEnvironment.SunIntensity, OnChanged);
	Properties->AddVecEntry("Sky color", Target->SceneEnvironment.SkyColor, OnChanged, true);
	Properties->AddVecEntry("Ground color", Target->SceneEnvironment.GroundColor, OnChanged, true);
	Properties->AddFloatEntry("Ambient intensity", Target->SceneEnvironment.AmbientIntensity, OnChanged);

	Properties->CreateNewHeading("Atmosphere");

	Properties->AddVecEntry("Fog Color", Target->SceneEnvironment.FogColor, OnChanged, true);
	Properties->AddFloatEntry("Fog range", Target->SceneEnvironment.FogRange, OnChanged);
	Properties->AddFloatEntry("Fog start", Target->SceneEnvironment.FogStart, OnChanged);
}

void engine::editor::ScenePanel::OnThemeChanged()
{
	OnResized();
}

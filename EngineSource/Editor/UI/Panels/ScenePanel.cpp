#include "ScenePanel.h"

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

	Properties->CreateNewHeading("Scene - " + Target->Name);

	Properties->CreateNewHeading("Lighting");
	Properties->AddVecEntry("Sun color", Target->SceneEnvironment.SunColor, nullptr, true);
	Properties->AddFloatEntry("Sun intensity", Target->SceneEnvironment.SunIntensity, nullptr);
	Properties->AddVecEntry("Sky color", Target->SceneEnvironment.SkyColor, nullptr, true);
	Properties->AddVecEntry("Ground color", Target->SceneEnvironment.GroundColor, nullptr, true);
	Properties->AddFloatEntry("Ambient intensity", Target->SceneEnvironment.AmbientIntensity, nullptr);

	Properties->CreateNewHeading("Atmosphere");

	Properties->AddVecEntry("Fog Color", Target->SceneEnvironment.FogColor, nullptr, true);
	Properties->AddFloatEntry("Fog range", Target->SceneEnvironment.FogRange, nullptr);
	Properties->AddFloatEntry("Fog start", Target->SceneEnvironment.FogStart, nullptr);
}

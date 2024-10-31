#include "SceneSubsystem.h"

engine::subsystem::SceneSubsystem::SceneSubsystem()
	: ISubsystem("Scene")
{
	LoadedScenes.push_back(new Scene());
	Main = LoadedScenes[0];
}

engine::subsystem::SceneSubsystem::~SceneSubsystem()
{
	for (Scene* scn : LoadedScenes)
	{
		delete scn;
	}
	LoadedScenes.clear();
}

void engine::subsystem::SceneSubsystem::Update()
{
	for (Scene* scn : LoadedScenes)
	{
		scn->Update();
	}
}

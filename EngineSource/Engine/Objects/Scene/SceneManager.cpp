#include "SceneManager.h"

using namespace engine;

engine::SceneManager::SceneManager()
{
}

engine::SceneManager::~SceneManager()
{
}

void engine::SceneManager::Update()
{
}

Scene* engine::SceneManager::GetScene()
{
	return ManagedScene;
}

#include <KlemmginePlugin.hpp>
#include <Engine/Objects/SceneObject.h>

class PluginObject : public engine::SceneObject
{
public:
	void Begin() override
	{

	}

	void Update() override
	{
	}

	void OnDestroyed() override
	{
	}
};

static engine::ObjectTypeID PluginObjectType = 0;

ENGINE_EXPORT void RegisterTypes()
{
	PluginObjectType = engine::RegisterObject("Cool Plugin Object", []() {return new PluginObject(); });
}

ENGINE_EXPORT void OnSceneLoaded(engine::Scene* New)
{
}
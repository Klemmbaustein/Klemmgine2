#pragma once
#include <Core/Transform.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>

namespace engine
{
	class Scene;

	class SceneManager : public ReflectionObject
	{
	public:

		ENGINE_OBJECT(SceneManager, "ReflectionObject", "Engine/Scene");

		SceneManager();
		~SceneManager();

		virtual void Update();

		Transform SceneTransform;

		friend class Scene;

		Scene* GetScene();

	private:
		Scene* ManagedScene = nullptr;
	};
}
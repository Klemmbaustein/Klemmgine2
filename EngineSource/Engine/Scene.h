#pragma once
#include "Graphics/Framebuffer.h"
#include "Graphics/Camera.h"
#include "Objects/SceneObject.h"

namespace engine
{
	class Scene
	{
	public:
		Scene();

		void Draw();
		void Update();

		graphics::Framebuffer* Buffer = nullptr;
		graphics::Camera* Cam = nullptr;
		std::vector<SceneObject*> Objects;

		static Scene* GetMain();

		template<typename T>
		T* CreateObject()
		{
			T* New = SceneObject::New<T>(this);
			this->Objects.push_back(New);
			return New;
		}

		void Save(string FileName);
	private:
		SerializedValue GetSceneInfo();
	};
}
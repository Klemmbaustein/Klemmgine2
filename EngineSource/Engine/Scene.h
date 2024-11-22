#pragma once
#include "Graphics/Framebuffer.h"
#include "Graphics/Camera.h"
#include "Objects/SceneObject.h"
#include <kui/Vec2.h>
#include <atomic>

namespace engine
{
	class Scene
	{
	public:
		explicit Scene(bool DoLoadAsync = false);
		Scene(string FilePath);
		Scene(const char* FilePath);

		void Draw();
		void Update();

		graphics::Framebuffer* Buffer = nullptr;
		graphics::Camera* Cam = nullptr;
		std::vector<SceneObject*> Objects;

		static Scene* GetMain();

		void OnResized(kui::Vec2ui NewSize);

		template<typename T>
		T* CreateObject()
		{
			T* New = SceneObject::New<T>(this, true);
			this->Objects.push_back(New);
			return New;
		}

		static std::atomic<int32> AsyncLoads;

		string Name;

		void LoadAsync(string SceneFile);
		void LoadAsyncFinish();

		void Save(string FileName);
	private:
		void LoadInternal(string File, bool Async);
		void Init();
		SerializedValue GetSceneInfo();
	};
}
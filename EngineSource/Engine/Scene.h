#pragma once
#include "Graphics/Framebuffer.h"
#include "Graphics/Camera.h"
#include "Objects/SceneObject.h"
#include <kui/Vec2.h>
#include <atomic>
#include <set>

namespace engine
{
	/**
	* @brief
	* A scene is a collection of objects and rendering information.
	* 
	* Each scene can have one graphics::Camera
	*/
	class Scene
	{
	public:
		Scene(bool DoLoadAsync = false);
		Scene(string FilePath);
		Scene(const char* FilePath);
		~Scene();

		Scene(const Scene&) = delete;

		void Draw();
		void Update();

		graphics::Framebuffer* Buffer = nullptr;
		graphics::Camera* SceneCamera = nullptr;
		graphics::Camera* UsedCamera = nullptr;
		std::vector<SceneObject*> Objects;

		kui::Vec2ui BufferSize;
		bool Resizable = true;

		/**
		* @brief
		* Gets the main scene.
		*/
		[[nodiscard]]
		static Scene* GetMain();

		void OnResized(kui::Vec2ui NewSize);

		/**
		* @brief
		*/
		template<typename T>
		T* CreateObject()
		{
			T* New = SceneObject::New<T>(this, true);
			this->Objects.push_back(New);
			return New;
		}

		/// The number of async scene loading operations in progress.
		static std::atomic<int32> AsyncLoads;

		/**
		* @brief
		* A string representing the scene.
		* 
		* Usually this is the path to the scene file (kts or kbs) that this scene was loaded from.
		*/
		string Name;

		/**
		* @brief
		* Starts asynchronously loading this scene from a file.
		*/
		void LoadAsync(string SceneFile);
		void LoadAsyncFinish();

		void Save(string FileName);

		bool ObjectDestroyed(SceneObject* Target) const;

		void PreLoadAsset(AssetRef Target);
	private:
		friend class SceneObject;
		std::vector<AssetRef> ReferencedAssets;
		std::set<SceneObject*> DestroyedObjects;
		void LoadInternal(string File, bool Async);
		void Init();
		SerializedValue GetSceneInfo();
	};
}
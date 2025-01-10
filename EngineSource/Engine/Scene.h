#pragma once
#include "Graphics/Framebuffer.h"
#include "Graphics/Camera.h"
#include "Graphics/Effects/CascadedShadows.h"
#include "Objects/SceneObject.h"
#include <kui/Vec2.h>
#include <atomic>
#include <set>
#include <mutex>
#include <Engine/Physics/Physics.h>
#include "Objects/Components/DrawableComponent.h"

namespace engine
{
	/**
	* @brief
	* A scene is a collection of objects and rendering information.
	* 
	* Each scene can have one graphics::Camera
	*/
	class Scene : ISerializable
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

		physics::PhysicsManager Physics = physics::PhysicsManager(this);
		graphics::CascadedShadows Shadows;

		/**
		* @brief
		*/
		template<typename T>
		T* CreateObject(Vector3 Position = 0, Rotation3 Rotation = 0, Vector3 Scale = 1)
		{
			T* New = SceneObject::New<T>(this, Position, Rotation, Scale, true);
			this->Objects.push_back(New);
			return New;
		}

		void CreateObjectFromID(uint32 ID, Vector3 Position = 0, Rotation3 Rotation = 0, Vector3 Scale = 1);

		/**
		* @brief
		* Reloads all objects in this scene.
		* 
		* This is used to transition from the editor to the game when built with the KLEMMGINE_EDITOR argument.
		*/
		void ReloadObjects(SerializedValue* FromState);

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

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		void Save(string FileName);

		bool ObjectDestroyed(SceneObject* Target) const;

		void AddDrawnComponent(DrawableComponent* New);
		void RemoveDrawnComponent(DrawableComponent* Removed);

		void PreLoadAsset(AssetRef Target);
	private:
		void DeSerializeInternal(SerializedValue* From, bool Async);
		friend class SceneObject;
		std::vector<AssetRef> ReferencedAssets;
		std::set<SceneObject*> DestroyedObjects;
		void LoadInternal(string File, bool Async);
		void Init();
		SerializedValue GetSceneInfo();

		struct SortingInfo
		{
			Vector3 Position;
			graphics::BoundingBox Bounds;
		};
		std::vector<std::pair<SortingInfo, DrawableComponent*>> SortedComponents;
		std::vector<DrawableComponent*> DrawnComponents;
		std::mutex DrawSortMutex;
		bool IsSorting = false;

		void StartSorting();
	};
}
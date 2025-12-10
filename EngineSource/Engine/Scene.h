#if !defined(ENGINE_PLUGIN) && !defined(ENGINE_UTILS_LIB)
#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Effects/CascadedShadows.h"
#include "Graphics/Effects/PostProcess.h"
#include "Graphics/Environment.h"
#include "Graphics/Framebuffer.h"
#include "Objects/Components/DrawableComponent.h"
#include "Objects/SceneObject.h"
#include <atomic>
#include <Engine/Physics/Physics.h>
#include <kui/Vec2.h>
#include <mutex>
#include <set>

namespace engine
{
	/**
	* @brief
	* A scene is a collection of objects and rendering information.
	*
	* Each scene can have one graphics::Camera
	*
	* @see subsystem::SceneSubsystem
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

		/**
		* @brief
		* A camera owned by this scene that is used if no other camera is available.
		*
		* This camera is also used by the editor as the editor camera.
		*
		* @see UsedCamera
		*/
		graphics::Camera* SceneCamera = nullptr;

		/**
		* @brief
		* The camera that is used to draw this scene.
		*
		* When the scene is first loaded, this has the same value as SceneCamera.
		*
		* @see CameraComponent
		*/
		graphics::Camera* UsedCamera = nullptr;
		graphics::Environment SceneEnvironment;

		/**
		* @brief
		* All objects in this scene.
		*
		* @see SceneObject
		*/
		std::vector<SceneObject*> Objects;

		kui::Vec2ui BufferSize;
		bool Resizable = true;

		/**
		* @brief
		* If this is true, this scene will be drawn each frame.
		*
		* @see engine::Scene::Redraw
		*/
		bool AlwaysRedraw = true;
		/**
		* @brief
		* Should this scene be redrawn the next frame.
		*
		* This value is ignored if AlwaysRedraw is true.
		* Otherwise this scene will only be drawn if it is true.
		*/
		bool Redraw = false;

		/**
		* @brief
		* Gets the main scene, or nullptr if there is none.
		*
		* The function might also return nullptr when:
		* -	The main scene is still loading.
		* - The SceneSubsystem is unloaded.
		*/
		[[nodiscard]]
		static Scene* GetMain();

		void OnResized(kui::Vec2ui NewSize);

		physics::PhysicsManager Physics = physics::PhysicsManager(this);
		graphics::CascadedShadows Shadows;
		graphics::PostProcess PostProcess;

		/**
		* @brief
		* Creates an object with the given type and places it into this scene.
		*
		* @see SceneObject
		*/
		template<typename T>
		T* CreateObject(Vector3 Position = 0, Rotation3 Rotation = 0, Vector3 Scale = 1)
		{
			T* New = SceneObject::New<T>(this, Position, Rotation, Scale, true);
			this->Objects.push_back(New);
			return New;
		}

		/**
		* @brief
		* Creates an object with the given type ID and places it into the scene.
		*
		* @see SceneObject
		*/
		engine::SceneObject* CreateObjectFromID(int32 ID, Vector3 Position = 0, Rotation3 Rotation = 0, Vector3 Scale = 1);

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

		/**
		* @brief
		* Saves this scene to a file.
		*/
		void Save(string FileName);
		/**
		* @brief
		* Saves this scene to a file.
		*/
		string SaveToString(string FileName);

		bool ObjectDestroyed(SceneObject* Target) const;

		/**
		* @brief
		* Adds a component to the list of drawn components.
		*
		* Drawn components will be sorted and rendered each time this scene is drawn.
		*
		* @see RemoveDrawnComponent
		* @see DrawableComponent
		*/
		void AddDrawnComponent(DrawableComponent* New);
		/**
		* @brief
		* Removes a component to the list of drawn components.
		*
		* @see AddDrawnComponent
		* @see DrawableComponent
		*/
		void RemoveDrawnComponent(DrawableComponent* Removed);

		/**
		* @brief
		* Pre-loads an asset from the given asset reference.
		*
		* All assets that are pre-loaded by this scene will be unloaded again when it is unloaded.
		* This function is meant to be used when loading the scene.
		* If the scene is loaded asynchronously using subsystem::SceneSubsystem::LoadSceneAsync(),
		* the asset will also be loaded asynchronously.
		*
		* @see subsystem::SceneSubsystem::LoadSceneAsync()
		* @see subsystem::SceneSubsystem
		*/
		void PreLoadAsset(AssetRef Target);

		uint32 GetDrawBuffer()
		{
			return SceneTexture;
		}

	private:

		struct SceneAsset
		{
			AssetRef FileReference;
			const void* LoadedData = nullptr;
		};

		void DeSerializeInternal(SerializedValue* From, bool Async);
		friend class SceneObject;
		std::vector<SceneAsset> ReferencedAssets;
		std::set<SceneObject*> DestroyedObjects;
		void LoadInternal(string File, bool Async);
		void Init();

		void UnloadAsset(const SceneAsset& Target);

		uint32 SceneTexture = 0;

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
#endif
#if !defined(ENGINE_PLUGIN)
#pragma once
#include "Objects/SceneObject.h"
#include <atomic>
#include <Engine/Physics/Physics.h>
#include <set>
#include <Engine/Sound/Sound.h>
#include "Graphics/Scene/GraphicsScene.h"
#include <Engine/Objects/Scene/SceneManager.h>

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
		void Update();

		/**
		* @brief
		* All objects in this scene.
		*
		* @see SceneObject
		*/
		std::vector<SceneObject*> Objects;

		sound::SoundContext* Sound = nullptr;

		graphics::GraphicsScene Graphics;

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

		physics::PhysicsManager Physics = physics::PhysicsManager(this);

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

		SceneManager* Manager = nullptr;
	private:

		struct SceneAsset
		{
			AssetRef FileReference;
			void* LoadedData = nullptr;
		};

		void DeSerializeInternal(SerializedValue* From, bool Async);
		friend class SceneObject;
		std::vector<SceneAsset> ReferencedAssets;
		std::set<SceneObject*> DestroyedObjects;
		void LoadInternal(string File, bool Async);
		void Init();

		void UnloadAsset(const SceneAsset& Target);

		SerializedValue GetSceneInfo();
	};
}
#endif
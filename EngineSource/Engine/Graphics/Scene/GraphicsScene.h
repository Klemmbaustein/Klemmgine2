#pragma once
#include <Engine/Graphics/Camera.h>
#include <Engine/Graphics/Effects/CascadedShadows.h>
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Graphics/Light/LightBvh.h>
#include <Engine/Graphics/Scene/BoundsHirarchy.h>
#include <Engine/Graphics/Framebuffer.h>
#include <set>
#include <memory>
#include <mutex>
#include <Engine/Objects/Components/DrawableComponent.h>
#include <kui/Vec2.h>

namespace engine::graphics
{
	class GraphicsScene
	{
	public:

		GraphicsScene();
		~GraphicsScene();

		void OnResized(kui::Vec2ui NewSize);

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

		void Init();

		/**
		* @brief
		* Removes a component to the list of drawn components.
		*
		* @see AddDrawnComponent
		* @see DrawableComponent
		*/
		void RemoveDrawnComponent(DrawableComponent* Removed);

		uint32 GetDrawBuffer()
		{
			return SceneTexture;
		}

		void Draw();

		Framebuffer* Buffer = nullptr;
		debug::DebugDraw Debug;

		/**
		* @brief
		* A camera owned by this scene that is used if no other camera is available.
		*
		* This camera is also used by the editor as the editor camera.
		*
		* @see UsedCamera
		*/
		Camera* SceneCamera = nullptr;

		/**
		* @brief
		* The camera that is used to draw this scene.
		*
		* When the scene is first loaded, this has the same value as SceneCamera.
		*
		* @see CameraComponent
		*/
		Camera* UsedCamera = nullptr;
		Environment SceneEnvironment;
		CascadedShadows Shadows;
		PostProcess Post;
		LightBvh Lights;

		kui::Vec2ui BufferSize;
		bool Resizable = true;

		/**
		* @brief
		* If this is true, this scene will be drawn each frame.
		*
		* @see engine::Scene::RedrawNextFrame
		*/
		bool AlwaysRedraw = true;
		/**
		* @brief
		* Should this scene be redrawn the next frame.
		*
		* This value is ignored if AlwaysRedraw is true.
		* Otherwise this scene will only be drawn if it is true.
		*/
		bool RedrawNextFrame = false;
		std::shared_ptr<graphics::BvhNode<DrawableComponent*>> DrawableHierarchy = nullptr;
		std::mutex HierarchyMutex;
		std::set<DrawableComponent*> RemovedDrawables;
		std::vector<DrawableComponent*> NewDrawables;

	private:

		uint32 SceneTexture = 0;

		struct SortingInfo
		{
			Vector3 Position;
			BoundingBox Bounds;
		};

		std::vector<DrawableComponent*> DrawnComponents;
		bool RebuildingHierarchy = false;

		void UpdateEnvironment();

		void ShadowDrawPass();
		void MainDrawPass();

		void BuildBoundingVolume();

	};
}
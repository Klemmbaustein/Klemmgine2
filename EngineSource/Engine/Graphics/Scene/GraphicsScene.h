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

		struct DrawableData
		{
			bool operator==(const DrawableData& other) const noexcept
			{
				return other.Id == Id;
			}
			bool operator<(const DrawableData& other) const noexcept
			{
				return other.Id < Id;
			}

			uint64 Id = 0;
			DrawableComponent* Component = nullptr;
		};
		std::shared_ptr<graphics::BvhNode<DrawableData>> DrawableHierarchy = nullptr;
		std::shared_ptr<std::mutex> HierarchyMutex = std::make_shared<std::mutex>();
		std::set<uint64> RemovedDrawableIds;
		std::vector<DrawableData> NewDrawables;

	private:

		uint32 SceneTexture = 0;

		struct SortingInfo
		{
			Vector3 Position;
			BoundingBox Bounds;
		};

		std::vector<DrawableData> DrawnComponents;
		bool RebuildingHierarchy = false;

		std::shared_ptr<bool> StopAsyncProcesses = std::make_shared<bool>(false);

		void UpdateEnvironment();

		void ShadowDrawPass();
		void MainDrawPass();

		void BuildBoundingVolume();

	};
}
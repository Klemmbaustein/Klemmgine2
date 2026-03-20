#pragma once
#include "DrawableComponent.h"
#include <Engine/File/AssetRef.h>
#include <Engine/File/ModelData.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Debug/DebugDraw.h>

namespace engine
{
	class MeshComponent : public DrawableComponent
	{
	public:

		std::vector<graphics::Material*> Materials;
		GraphicsModel* DrawnModel = nullptr;
		bool DrawAsOpaqueStencil = false;

		virtual void Update() override;

		virtual void Draw(graphics::Camera* From, graphics::GraphicsScene* In) override;
		virtual void SimpleDraw(graphics::ShaderObject* With) override;

		void Load(AssetRef From, bool LoadMaterials = true);
		void Load(GraphicsModel* From);

		void OnAttached() override;

		~MeshComponent() override;

		void UpdateTransform(bool IsDirty) override;

		void ClearModel(bool RemoveDrawnComponent);
	private:

		debug::DebugBox* Debug = nullptr;

		void InitializeModel();

		bool IsRegistered = false;
	};
}
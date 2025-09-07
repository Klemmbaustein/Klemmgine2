#pragma once
#include "DrawableComponent.h"
#include <Engine/File/AssetRef.h>
#include <Engine/File/ModelData.h>
#include <Engine/Graphics/Material.h>

namespace engine
{
	class MeshComponent : public DrawableComponent
	{
	public:

		std::vector<graphics::Material*> Materials;
		GraphicsModel* DrawnModel = nullptr;

		virtual void Update() override;

		virtual void Draw(graphics::Camera* From) override;
		virtual void SimpleDraw(graphics::ShaderObject* With) override;

		void Load(AssetRef From);

		void OnAttached() override;

		~MeshComponent() override;

		void ClearModel(bool RemoveDrawnComponent);
	private:
		bool IsRegistered = false;
	};
}
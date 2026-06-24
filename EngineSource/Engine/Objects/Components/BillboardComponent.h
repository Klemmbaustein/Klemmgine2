#pragma once
#include <Engine/Objects/Components/DrawableComponent.h>
#include <Engine/File/ModelData.h>

namespace engine
{
	class BillboardComponent : public DrawableComponent
	{
	public:

		BillboardComponent();

		void OnAttached() override;
		void OnDetached() override;
		bool UpdateTransform(bool IsDirty) override;

		void LoadImage(AssetRef Image);
		void SetColor(Vector3 NewColor);

		// Inherited via DrawableComponent
		void Draw(graphics::Camera* From, graphics::GraphicsScene* In) override;
		void DrawTransparent(graphics::Camera* From, graphics::GraphicsScene* In) override;

	private:
		GraphicsModel* Model = nullptr;
		std::vector<graphics::Material*> Materials;
	};
}
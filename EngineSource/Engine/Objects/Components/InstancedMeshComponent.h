#pragma once
#include <Engine/Objects/Components/MeshComponent.h>

namespace engine
{
	class InstancedMeshComponent : public MeshComponent
	{
	public:

		InstancedMeshComponent();
		~InstancedMeshComponent();

		void OnAttached() override;

		void Draw(graphics::Renderer* Render, graphics::Camera* From, graphics::GraphicsScene* In) override;
		void SimpleDraw(graphics::Renderer* Render, graphics::ShaderObject* With) override;

		bool UpdateTransform(bool IsDirty) override;
		void InitializeModel() override;

		std::vector<Transform> Instances;

	private:
		void InitBounds();

		BoundingBox InstancesBounds;

		bool Attached = false;
		graphics::Model* InstancedModel = nullptr;
		graphics::VertexArrayData* InstanceArray = nullptr;
	};
}
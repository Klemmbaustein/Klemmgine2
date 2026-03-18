#pragma once
#include <Engine/Graphics/Model.h>
#include <Engine/Graphics/Material.h>
#include <list>

namespace engine::graphics
{
	class GraphicsScene;
}

namespace engine::debug
{
	class DebugDraw;

	class DebugShape
	{
	public:
		DebugShape() = default;
		virtual ~DebugShape();
		virtual void Draw(graphics::GraphicsScene* With) = 0;

		DebugDraw* Owner = nullptr;
	};

	class DebugBox : public DebugShape
	{
	public:
		DebugBox(Vector3 Position, Rotation3 Rotation, Vector3 Scale, Vector3 Color);
		DebugBox(Transform WithTransform, Vector3 Color);
		~DebugBox();
		void Draw(graphics::GraphicsScene* With) override;

		graphics::Model* CubeModel = nullptr;
		graphics::Material* CubeMaterial;
		Vector3 CubeColor;
		Transform CubeTransform;

		void SetColor(Vector3 NewColor);
	};

	class DebugDraw
	{
	public:
		DebugDraw();
		~DebugDraw();

		void Draw(graphics::GraphicsScene* With);

		void AddShape(DebugShape* Shape);
		void RemoveShape(DebugShape* Shape);

	private:
		std::list<DebugShape*> Shapes;
	};
}
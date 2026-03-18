#pragma once
#include <Engine/Graphics/Camera.h>
#include <Engine/Objects/Components/DrawableComponent.h>
#include <Engine/Graphics/Framebuffer.h>
#include <set>

namespace engine::graphics
{
	class GraphicsScene;

	class CascadedShadows
	{
	public:

		CascadedShadows();

		void Init();
		void Update(Camera* From);
		uint32 Draw(GraphicsScene* With);
		void BindUniforms(ShaderObject* Target) const;

		Vector3 LightDirection = Vector3(1, 2, 1).Normalize();

		static uint32 LightDepthMaps;

		bool Enabled = false;

	private:
		bool EnvironmentHasShadows = true;

		mutable std::set<const ShaderObject*> LastShaders;

		bool ShouldRender() const
		{
			return Enabled && EnvironmentHasShadows;
		}

		float BiasModifier = 0;
		static graphics::ShaderObject* ShadowShader;
		static uint32 MatricesBuffer;
		static uint32 LightFBO;

		struct MatrixEntry
		{
			glm::mat4 Matrix;
			BoundingBox Bounds;
		};

		std::vector<glm::mat4> Matrices;
		std::vector<BoundingBox> CascadeBounds;

		MatrixEntry GetLightSpaceMatrix(graphics::Camera* From, float NearPlane, float FarPlane);
		std::vector<MatrixEntry> GetLightSpaceMatrices(graphics::Camera* From);
		std::vector<glm::vec4> GetFrustumCornersWorldSpace(glm::mat4 ViewProjection);
	};
}
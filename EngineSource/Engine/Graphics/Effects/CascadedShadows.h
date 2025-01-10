#pragma once
#include <Engine/Graphics/Camera.h>
#include <Engine/Objects/Components/DrawableComponent.h>
#include <Engine/Graphics/Framebuffer.h>

namespace engine::graphics
{
	class CascadedShadows
	{
	public:

		CascadedShadows();

		void Init();

		void Update(Camera* From);

		uint32 Draw(std::vector<DrawableComponent*> Components);

		Vector3 LightDirection = Vector3(1, 4, 1).Normalize();

		static uint32 LightDepthMaps;

		void BindUniforms(graphics::ShaderObject* Target);

	private:
		float BiasModifier = 0;
		static graphics::ShaderObject* ShadowShader;
		static uint32 MatricesBuffer;
		static uint32 LightFBO;
		std::vector<glm::mat4> LightMatrices;
		glm::mat4 GetLightSpaceMatrix(graphics::Camera* From, float NearPlane, float FarPlane);
		std::vector<glm::mat4> GetLightSpaceMatrices(graphics::Camera* From);
		std::vector<glm::vec4> GetFrustumCornersWorldSpace(glm::mat4 ViewProjection);
	};
}
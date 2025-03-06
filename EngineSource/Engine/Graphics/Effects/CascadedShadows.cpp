#include "CascadedShadows.h"
#include <glm/vec4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <Engine/Internal/OpenGL.h>
#include <Core/Log.h>
#include <Engine/File/Resource.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/OpenGL.h>
#include <iostream>
using namespace engine;
using namespace engine::graphics;

constexpr uint32 ShadowResolution = 1000;
constexpr float CAMERA_FAR_PLANE = 200.0f;
std::vector<float> ShadowCascadeLevels = { CAMERA_FAR_PLANE / 30.0f, CAMERA_FAR_PLANE / 8.0f, CAMERA_FAR_PLANE / 2.0f };

uint32 CascadedShadows::LightFBO = 0;
uint32 CascadedShadows::LightDepthMaps = 0;
uint32 CascadedShadows::MatricesBuffer = 0;
graphics::ShaderObject* CascadedShadows::ShadowShader = nullptr;

CascadedShadows::CascadedShadows()
{
	if (openGL::GetGLVersion() >= openGL::Version::GL430 || glewIsSupported("GL_ARB_uniform_buffer_object"))
	{
		Enabled = true;
	}
}

void CascadedShadows::Init()
{
	if (!Enabled)
		return;

	if (ShadowShader)
		return;

	ShadowShader = new ShaderObject(
		resource::GetTextFile("res:shader/internal/shadow.vert"),
		resource::GetTextFile("res:shader/internal/shadow.frag"),
		resource::GetTextFile("res:shader/internal/shadow.geom")
	);

	glGenFramebuffers(1, &LightFBO);

	glGenTextures(1, &LightDepthMaps);
	glBindTexture(GL_TEXTURE_2D_ARRAY, LightDepthMaps);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_DEPTH_COMPONENT32F,
		ShadowResolution,
		ShadowResolution,
		GLsizei(ShadowCascadeLevels.size()) + 1,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float BorderColor[] = { 1.0f, 0, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BorderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, LightDepthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::Error("Failed to create the shadow framebuffer!");
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGenBuffers(1, &MatricesBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, MatricesBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	ShadowShader->Bind();
	ShadowShader->SetInt(ShadowShader->GetUniformLocation("u_shadowCascadeCount"), uint32(ShadowCascadeLevels.size() + 1));
}

void engine::graphics::CascadedShadows::Update(Camera* From)
{
	if (!Enabled)
		return;
	LightMatrices = GetLightSpaceMatrices(From);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesBuffer);
	for (size_t i = 0; i < LightMatrices.size(); ++i)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &LightMatrices[i]);
		auto vec = LightMatrices[i] * glm::vec4(3, 0, 0, 1);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	BiasModifier = Vector3::Dot(Vector3::Forward(From->Rotation), LightDirection);
}

uint32 CascadedShadows::Draw(std::vector<DrawableComponent*> Components)
{
	if (!Enabled)
		return 0;

	glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
	glViewport(0, 0, ShadowResolution, ShadowResolution);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShadowShader->Bind();
	for (DrawableComponent* i : Components)
	{
		i->SimpleDraw(ShadowShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return LightDepthMaps;
}

void engine::graphics::CascadedShadows::BindUniforms(graphics::ShaderObject* Target) const
{
	if (!Target || !Enabled)
		return;

	for (size_t i = 0; i < ShadowCascadeLevels.size(); ++i)
	{
		Target->SetFloat(Target->GetUniformLocation("cascadePlaneDistances[" + std::to_string(i) + "]"), ShadowCascadeLevels[i]);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, graphics::CascadedShadows::LightDepthMaps);
	Target->SetInt(Target->GetUniformLocation("u_shadowMaps"), 0);
	Target->SetFloat(Target->GetUniformLocation("u_shadowBiasModifier"), BiasModifier);
	Target->SetVec3(Target->GetUniformLocation("u_lightDirection"), LightDirection);
	Target->SetInt(Target->GetUniformLocation("u_shadowCascadeCount"), uint32(ShadowCascadeLevels.size() + 1));
}

glm::mat4 CascadedShadows::GetLightSpaceMatrix(graphics::Camera* From, float NearPlane, float FarPlane)
{
	glm::mat4 CameraProjection = glm::perspective(From->FOV, From->Aspect, NearPlane, FarPlane);
	auto Corners = GetFrustumCornersWorldSpace(CameraProjection * From->View);

	glm::vec3 FrustumCenter = glm::vec3(0, 0, 0);
	for (const auto& v : Corners)
	{
		FrustumCenter += glm::vec3(v);
	}
	FrustumCenter /= float(Corners.size());

	const float SnapSize = FarPlane / 5;

	FrustumCenter.x = std::floor(FrustumCenter.x / SnapSize) * SnapSize;
	FrustumCenter.y = std::floor(FrustumCenter.y / SnapSize) * SnapSize;
	FrustumCenter.z = std::floor(FrustumCenter.z / SnapSize) * SnapSize;

	const auto LightView = glm::lookAt(
		FrustumCenter + glm::vec3(LightDirection.X, LightDirection.Y, LightDirection.Z),
		FrustumCenter,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	constexpr float FloatMin = std::numeric_limits<float>::lowest();
	constexpr float FloatMax = std::numeric_limits<float>::max();

	float MinX = FloatMax;
	float MaxX = FloatMin;
	float MinY = FloatMax;
	float MaxY = FloatMin;
	float MinZ = FloatMax;
	float MaxZ = FloatMin;

	for (const auto& v : Corners)
	{
		const auto trf = LightView * v;
		MinX = std::min(MinX, trf.x - SnapSize);
		MaxX = std::max(MaxX, trf.x + SnapSize);
		MinY = std::min(MinY, trf.y - SnapSize);
		MaxY = std::max(MaxY, trf.y + SnapSize);
		MinZ = std::min(MinZ, trf.z - SnapSize);
		MaxZ = std::max(MaxZ, trf.z + SnapSize);
	}


	MinX = std::floor(MinX / SnapSize) * SnapSize;
	MinY = std::floor(MinY / SnapSize) * SnapSize;
	MinZ = std::floor(MinZ / SnapSize) * SnapSize;
	MaxX = std::floor(MaxX / SnapSize) * SnapSize;
	MaxY = std::floor(MaxY / SnapSize) * SnapSize;
	MaxZ = std::floor(MaxZ / SnapSize) * SnapSize;

	constexpr float DepthMultiplier = 20;
	if (MinZ < 0)
	{
		MinZ *= DepthMultiplier;
	}
	else
	{
		MinZ /= DepthMultiplier;
	}
	if (MaxZ < 0)
	{
		MaxZ /= DepthMultiplier;
	}
	else
	{
		MaxZ *= DepthMultiplier;
	}
	const glm::mat4 LightProjection = glm::ortho(MinX, MaxX, MinY, MaxY, MinZ, MaxZ);

	return LightProjection * LightView;
}

std::vector<glm::mat4> CascadedShadows::GetLightSpaceMatrices(graphics::Camera* From)
{
	const float CameraNearPlane = 0.01f;

	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < ShadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(GetLightSpaceMatrix(From, CameraNearPlane, ShadowCascadeLevels[i]));
		}
		else if (i < ShadowCascadeLevels.size())
		{
			ret.push_back(GetLightSpaceMatrix(From, ShadowCascadeLevels[i - 1], ShadowCascadeLevels[i]));
		}
		else
		{
			ret.push_back(GetLightSpaceMatrix(From, ShadowCascadeLevels[i - 1], CAMERA_FAR_PLANE));
		}
	}
	return ret;
}

std::vector<glm::vec4> CascadedShadows::GetFrustumCornersWorldSpace(glm::mat4 ViewProjection)
{
	const glm::mat4 InverseViewProjection = glm::inverse(ViewProjection);

	std::vector<glm::vec4> FrustumCorners;
	FrustumCorners.reserve(2 * 2 * 2);
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = InverseViewProjection * glm::vec4(
					2.0f * x - 1.0f,
					2.0f * y - 1.0f,
					2.0f * z - 1.0f,
					1.0f
				);
				FrustumCorners.push_back(pt / pt.w);
			}
		}
	}
	return FrustumCorners;
}

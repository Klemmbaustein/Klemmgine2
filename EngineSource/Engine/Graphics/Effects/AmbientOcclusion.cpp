#include "AmbientOcclusion.h"
#include "PostProcess.h"
#include <algorithm>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Internal/OpenGL.h>
#include <random>
#include <Core/Log.h>
#include <Engine/Scene.h>

const size_t AO_SAMPLES = 24;

engine::graphics::AmbientOcclusion::AmbientOcclusion()
{
	DrawOrder = AO_DRAW_ORDER;

	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine RandomGenerator;

	for (size_t i = 0; i < AO_SAMPLES; i++)
	{
		Vector3 Sample = Vector3(
			RandomFloats(RandomGenerator) * 2.0f - 1.0f,
			RandomFloats(RandomGenerator) * 2.0f - 1.0f,
			RandomFloats(RandomGenerator)
		).Normalize();

		Sample *= Vector3(RandomFloats(RandomGenerator));
		float Scale = float(i) / float(AO_SAMPLES);

		// scale samples s.t. they're more aligned to center of kernel
		Scale = std::lerp(0.1f, 1.0f, Scale * Scale);
		Sample *= Scale;
		AoKernel.push_back(Sample);
	}

	std::vector<Vector3> AoNoise;
	for (size_t i = 0; i < 16; i++)
	{
		Vector3 Noise = Vector3(
			RandomFloats(RandomGenerator),
			RandomFloats(RandomGenerator),
			0.0f);
		AoNoise.push_back(Noise);
	}

	glGenTextures(1, &NoiseTexture);
	glBindTexture(GL_TEXTURE_2D, NoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 4, 4, 0, GL_RGB, GL_FLOAT, AoNoise.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	AoShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert", "res:shader/effects/ssao.frag");
	AoShader->Bind();
	for (size_t i = 0; i < AO_SAMPLES; i++)
	{
		glUniform3fv(AoShader->GetUniformLocation("samples[" + std::to_string(i) + "]"), 1, &AoKernel[i].X);
	}

	AoMergeShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert", "res:shader/effects/ssaoMerge.frag");
}

void engine::graphics::AmbientOcclusion::OnBufferResized(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;
	this->AoWidth = Width / 2;
	this->AoHeight = Height / 2;

	if (AoBuffer)
	{
		FreeBuffer(AoBuffer);
	}

	AoBuffer = CreateNewBuffer(AoWidth, AoHeight, false);
}

uint32 engine::graphics::AmbientOcclusion::Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Buffer->Textures[2]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Buffer->Textures[3]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NoiseTexture);

	AoShader->Bind();
	AoShader->SetInt(AoShader->GetUniformLocation("gPosition"), 0);
	AoShader->SetInt(AoShader->GetUniformLocation("gNormal"), 1);
	AoShader->SetInt(AoShader->GetUniformLocation("texNoise"), 2);
	AoShader->SetVec2(AoShader->GetUniformLocation("noiseScale"), Vector2(float(Width), float(Height)) / 4.0f);
	glUniformMatrix4fv(AoShader->GetUniformLocation("projection"), 1, false, &Cam->Projection[0][0]);

	uint32 AoTexture = DrawBuffer(AoBuffer, this->AoWidth, this->AoHeight);

	auto ResultBuffer = With->NextBuffer();
	
	glViewport(0, 0, Width, Height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, AoTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Buffer->Textures[2]);
	AoMergeShader->Bind();
	AoMergeShader->SetInt(AoMergeShader->GetUniformLocation("u_mainTexture"), 0);
	AoMergeShader->SetInt(AoMergeShader->GetUniformLocation("u_aoTexture"), 1);
	AoMergeShader->SetInt(AoMergeShader->GetUniformLocation("u_position"), 2);

	glBindFramebuffer(GL_FRAMEBUFFER, ResultBuffer.first);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	return ResultBuffer.second;

}

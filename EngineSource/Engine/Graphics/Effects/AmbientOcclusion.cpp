#include "AmbientOcclusion.h"
#include "PostProcess.h"
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <random>

using namespace engine::graphics;

const size_t AO_SAMPLES = 32;

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

	TextureOptions Options;
	Options.HasAlphaChannel = false;
	Options.Filter = TextureOptions::Nearest;
	Options.TextureBorders = TextureOptions::Repeat;
	Options.IsFloatTexture = true;
	NoiseTexture = Render->CreateTexture((uByte*)AoNoise.data(), 4, 4, Options);

	//glGenTextures(1, &NoiseTexture);
	//glBindTexture(GL_TEXTURE_2D, NoiseTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 4, 4, 0, GL_RGB, GL_FLOAT, AoNoise.data());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	AoShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert",
		"res:shader/effects/ssao.frag");
	AoShader->Bind();
	for (size_t i = 0; i < AO_SAMPLES; i++)
	{
		AoShader->SetVec3(AoShader->GetUniformLocation("samples[" + std::to_string(i) + "]"), AoKernel[i]);
	}

	AoMergeShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert",
		"res:shader/effects/ssaoMerge.frag");
}

void engine::graphics::AmbientOcclusion::OnBufferResized(uint32 Width, uint32 Height)
{
	this->Width = Width;
	this->Height = Height;
	this->AoWidth = Width / 2;
	this->AoHeight = Height / 2;

	if (AoBuffer)
	{
		delete AoBuffer;
	}

	AoBuffer = CreateNewBuffer(AoWidth, AoHeight, false);
}

RendererTexture* engine::graphics::AmbientOcclusion::Draw(RendererTexture* Texture, PostProcess* With,
	Framebuffer* Buffer, Camera* Cam)
{
	if (!Cam->UsedEnvironment->Render.AmbientOcclusion
		|| !VideoSubsystem::Current->DrawAmbientOcclusion)
	{
		return Texture;
	}

	auto Pass = StartRenderPass();
	Pass->UseShader(AoShader);
	Pass->BindTexture("gPosition", Buffer->Buffer->GetTexture(Framebuffer::TEXTURE_POSITION));
	Pass->BindTexture("gNormal", Buffer->Buffer->GetTexture(Framebuffer::TEXTURE_NORMAL));
	Pass->BindTexture("texNoise", NoiseTexture);

	AoShader->SetVec2(AoShader->GetUniformLocation("noiseScale"), Vector2(float(AoWidth), float(AoHeight)) / 4.0f);
	AoShader->SetMatrix(AoShader->GetUniformLocation("projection"), Cam->Projection);

	auto AoTexture = DrawBuffer(Pass, AoBuffer, this->AoWidth, this->AoHeight);

	auto ResultBuffer = With->NextBuffer();
	ResultBuffer->Activate();

	Pass->SetResolution(Width, Height);
	Pass->ResetTextures();
	Pass->UseShader(AoMergeShader);
	Pass->BindTexture("u_mainTexture", Texture);
	Pass->BindTexture("u_aoTexture", AoTexture);
	Pass->BindTexture("u_position", Buffer->Buffer->GetTexture(Framebuffer::TEXTURE_POSITION));

	Pass->DrawVertices(3);

	return ResultBuffer->GetTexture(0);
}

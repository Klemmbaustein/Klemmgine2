#include "Bloom.h"
#include <Engine/Graphics/ShaderLoader.h>
#include "PostProcess.h"

using namespace engine::graphics;

engine::graphics::Bloom::Bloom()
{
	DrawOrder = BLOOM_DRAW_ORDER;
	BloomBuffers[0] = 0;
	BloomBuffers[1] = 0;

	BloomShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert", "res:shader/effects/bloom.frag");
	BloomMergeShader = ShaderLoader::Current->Get("res:shader/internal/postProcess.vert", "res:shader/effects/bloomMerge.frag");
	BloomTextureLocation = BloomShader->GetUniformLocation("u_texture");
	TextureSizeLocation = BloomShader->GetUniformLocation("u_res");
}

engine::graphics::Bloom::~Bloom()
{
	delete BloomBuffers[0];
	delete BloomBuffers[1];
}

void engine::graphics::Bloom::OnBufferResized(uint32 Width, uint32 Height)
{
	if (BloomBuffers[0])
	{
		delete BloomBuffers[0];
		delete BloomBuffers[1];
	}
	BloomWidth = uint32(float(Width) * 0.2f);
	BloomHeight = uint32(float(Height) * 0.2f);
	this->Width = Width;
	this->Height = Height;
	BloomBuffers[0] = CreateNewBuffer(BloomWidth, BloomHeight, false);
	BloomBuffers[1] = CreateNewBuffer(BloomWidth, BloomHeight, false);
}

RendererTexture* engine::graphics::Bloom::Draw(RendererTexture* Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	if (!Cam->UsedEnvironment->Render.Bloom)
	{
		return Texture;
	}

	auto& Settings = Cam->UsedEnvironment->Render;
	auto Pass = StartRenderPass();

	Pass->SetResolution(BloomWidth, BloomHeight);

	Pass->UseShader(BloomShader);
	BloomShader->SetVec2(TextureSizeLocation, Vector2(1.0f) / Vector2(float(BloomWidth), float(BloomHeight)));

	size_t BloomAmount = Cam->UsedEnvironment->Render.BloomSamples;
	bool Horizontal = false;

	RendererTexture* IterationTexture = Texture;
	auto HorizontalLocation = BloomShader->GetUniformLocation("horizontal");
	for (size_t i = 0; i < BloomAmount; i++)
	{
		BloomBuffers[Horizontal]->Activate();
		BloomShader->SetInt(HorizontalLocation, (i % Settings.BloomShape));
		Pass->ResetTextures();
		Pass->BindTexture("u_texture", IterationTexture);
		Pass->DrawVertices(3);
		IterationTexture = BloomBuffers[Horizontal]->GetTexture(0);
		Horizontal = !Horizontal;
	}

	auto ResultBuffer = With->NextBuffer();
	Pass->SetResolution(Width, Height);

	Pass->ResetTextures();

	Pass->UseShader(BloomMergeShader);
	Pass->BindTexture("u_mainTexture", Texture);
	Pass->BindTexture("u_bloomTexture", IterationTexture);
	BloomMergeShader->SetFloat(BloomMergeShader->GetUniformLocation("u_bloomStrength"), Settings.BloomStrength);
	BloomMergeShader->SetFloat(BloomMergeShader->GetUniformLocation("u_bloomThreshold"), Settings.BloomThreshold);

	ResultBuffer->Activate();
	Pass->DrawVertices(3);

	return ResultBuffer->GetTexture(0);
}

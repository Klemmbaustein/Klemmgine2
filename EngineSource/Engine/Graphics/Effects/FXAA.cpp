#include "FXAA.h"
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Graphics/ShaderLoader.h>

using namespace engine::graphics;
using namespace engine;

engine::graphics::FXAA::FXAA()
{
	EffectShader = ShaderLoader::Current->Get(
		"res:shader/internal/postProcess.vert",
		"res:shader/effects/fxaa.frag");

	DrawOrder = AA_DRAW_ORDER;
}

engine::graphics::FXAA::~FXAA()
{
}

void engine::graphics::FXAA::OnBufferResized(uint32 Width, uint32 Height)
{
}

RendererTexture* engine::graphics::FXAA::Draw(RendererTexture* Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	if (!Cam->UsedEnvironment->Render.AntiAlias)
	{
		return Texture;
	}

	auto Pass = StartRenderPass();
	auto PassBuffer = With->NextBuffer();

	Pass->UseShader(EffectShader);
	Texture->SetFilterMode(TextureOptions::Linear, 0);
	Pass->BindTexture("u_texture", Texture);

	auto Result = DrawBuffer(Pass, PassBuffer, Buffer->Width, Buffer->Height);
	Texture->SetFilterMode(TextureOptions::Nearest, 0);

	return Result;
}

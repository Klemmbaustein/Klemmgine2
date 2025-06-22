#include "FXAA.h"
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Internal/OpenGL.h>

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

uint32 engine::graphics::FXAA::Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	std::pair<uint32, uint32> OutlineBuffer = With->NextBuffer();

	EffectShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glActiveTexture(GL_TEXTURE1);

	EffectShader->SetInt(EffectShader->GetUniformLocation("u_texture"), 0);

	glBindFramebuffer(GL_FRAMEBUFFER, OutlineBuffer.first);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

	return OutlineBuffer.second;
}

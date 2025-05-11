#include "Outline.h"
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Internal/OpenGL.h>

using namespace engine::graphics;
using namespace engine::editor;
using namespace engine;

engine::editor::EditorOutline::EditorOutline()
{
	OutlineShader = ShaderLoader::Current->Get(
		"res:shader/internal/postProcess.vert",
		"res:shader/internal/editorOutline.frag");

	DrawOrder = OUTLINE_DRAW_ORDER;
}

engine::editor::EditorOutline::~EditorOutline()
{
}

void engine::editor::EditorOutline::OnBufferResized(uint32 Width, uint32 Height)
{
}

uint32 engine::editor::EditorOutline::Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	std::pair<uint32, uint32> OutlineBuffer = With->NextBuffer();

	OutlineShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Buffer->Textures[Framebuffer::TEXTURE_DEPTH_STENCIL]);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

	OutlineShader->SetInt(OutlineShader->GetUniformLocation("u_texture"), 0);
	OutlineShader->SetInt(OutlineShader->GetUniformLocation("u_depthStencil"), 1);

	glBindFramebuffer(GL_FRAMEBUFFER, OutlineBuffer.first);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

	return OutlineBuffer.second;
}

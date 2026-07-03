#include "Outline.h"
#include <Engine/Graphics/Effects/PostProcess.h>
#include <Engine/Graphics/ShaderLoader.h>

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

RendererTexture* engine::editor::EditorOutline::Draw(RendererTexture* Texture, PostProcess* With,
	Framebuffer* Buffer, Camera* Cam)
{
	auto Pass = StartRenderPass();
	auto PassBuffer = With->NextBuffer();

	Pass->UseShader(OutlineShader);
	Pass->BindTexture("u_texture", Texture);
	Pass->BindTexture("u_depthStencil", Buffer->Buffer->GetStencilTexture());

	return DrawBuffer(Pass, PassBuffer, Buffer->Width, Buffer->Height);
}

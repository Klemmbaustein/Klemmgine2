#include "PostProcessEffect.h"
#include <Engine/Graphics/VideoSubsystem.h>

using namespace engine::graphics;

engine::graphics::PostProcessEffect::PostProcessEffect()
{
	Render = VideoSubsystem::Current->Renderer;
}

engine::graphics::PostProcessEffect::~PostProcessEffect()
{
}

void engine::graphics::PostProcessEffect::OnBufferResized(uint32 Width, uint32 Height)
{
}

RendererDrawTarget* engine::graphics::PostProcessEffect::CreateNewBuffer(uint32 Width, uint32 Height, bool FilterNearest)
{
	return Render->CreateDrawTarget(Width, Height, {
		DrawTargetBuffer{
			.FilterNearest = FilterNearest
		} });
}

DrawCommand* engine::graphics::PostProcessEffect::StartRenderPass()
{
	auto Pass = this->Render->StartRender();
	Pass->SetDepthCheckEnabled(false);
	Pass->SetFaceCullEnabled(false);
	return Pass;
}

RendererTexture* engine::graphics::PostProcessEffect::DrawBuffer(DrawCommand* Pass, RendererDrawTarget* Target,
	uint32 Width, uint32 Height)
{
	Target->Activate();
	Pass->SetResolution(Width, Height);
	Pass->DrawVertices(3);

	return Target->GetTexture(0);
}

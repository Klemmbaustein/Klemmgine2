#pragma once
#include <Engine/Graphics/Effects/PostProcessEffect.h>
#include <Engine/Graphics/ShaderObject.h>

namespace engine::editor
{
	class EditorOutline : public graphics::PostProcessEffect
	{
	public:

		EditorOutline();

		~EditorOutline() override;

		static constexpr int64 OUTLINE_DRAW_ORDER = 100;

		uint32 Draw(uint32 Texture, graphics::PostProcess* With, graphics::Framebuffer* Buffer, graphics::Camera* Cam) override;
		graphics::ShaderObject* OutlineShader = nullptr;
		void OnBufferResized(uint32 Width, uint32 Height) override;

	};
}
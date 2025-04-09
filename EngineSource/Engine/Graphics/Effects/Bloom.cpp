#include "Bloom.h"
#include <Engine/Graphics/ShaderLoader.h>
#include <Engine/Internal/OpenGL.h>
#include "PostProcess.h"

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
	FreeBuffer(BloomBuffers[0]);
	FreeBuffer(BloomBuffers[1]);
}

void engine::graphics::Bloom::OnBufferResized(uint32 Width, uint32 Height)
{
	if (BloomBuffers[0])
	{
		FreeBuffer(BloomBuffers[0]);
		FreeBuffer(BloomBuffers[1]);
	}
	BloomWidth = Width * 0.25f;
	BloomHeight = Height * 0.25f;
	this->Width = Width;
	this->Height = Height;
	BloomBuffers[0] = CreateNewBuffer(BloomWidth, BloomHeight, false);
	BloomBuffers[1] = CreateNewBuffer(BloomWidth, BloomHeight, false);
}

uint32 engine::graphics::Bloom::Draw(uint32 Texture, PostProcess* With, Framebuffer* Buffer, Camera* Cam)
{
	BloomShader->Bind();
	BloomShader->SetInt(BloomTextureLocation, 0);
	BloomShader->SetVec2(TextureSizeLocation, Vector2(1.0f) / Vector2(BloomWidth, BloomHeight));

	size_t BloomAmount = 12;
	bool Horizontal = false;

	uint32 IterationTexture = Texture;
	glActiveTexture(GL_TEXTURE0);

	auto HorizontalLocation = BloomShader->GetUniformLocation("horizontal");

	for (size_t i = 0; i < BloomAmount; i++)
	{
		BloomShader->SetInt(HorizontalLocation, Horizontal);
		glBindTexture(GL_TEXTURE_2D, IterationTexture);
		IterationTexture = DrawBuffer(BloomBuffers[Horizontal], BloomWidth, BloomHeight);
		Horizontal = !Horizontal;
	}

	auto ResultBuffer = With->NextBuffer();
	glViewport(0, 0, Width, Height);
	glBindTexture(GL_TEXTURE_2D, IterationTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Texture);

	BloomMergeShader->Bind();
	BloomMergeShader->SetInt(BloomMergeShader->GetUniformLocation("u_mainTexture"), 1);
	BloomMergeShader->SetInt(BloomMergeShader->GetUniformLocation("u_bloomTexture"), 0);

	glBindFramebuffer(GL_FRAMEBUFFER, ResultBuffer.first);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	return ResultBuffer.second;
}

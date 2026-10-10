#include "Model.h"
#include <Engine/File/ModelData.h>
#include <Engine/Scene.h>
#include <Engine/Graphics/VideoSubsystem.h>

engine::graphics::Model::Model(const ModelData* From)
{
	for (auto& i : From->Meshes)
	{
		ModelVertexBuffers.push_back(VideoSubsystem::Current->Renderer->CreateVertexBuffer(i.Vertices, i.Indices));
	}
}

engine::graphics::Model::~Model()
{
	for (VertexBuffer* i : ModelVertexBuffers)
	{
		delete i;
	}
}

void engine::graphics::Model::Draw(Renderer* Render, GraphicsScene* In, const Transform& At, graphics::Camera* With,
	std::vector<Material*>& UsedMaterials, const BoundingBox& Bounds, bool Stencil, bool IsTransparent)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		auto Pass = Render->StartRender();
		if (IsTransparent != UsedMaterials[i]->IsTransparent)
		{
			continue;
		}
		Pass->SetBlendEnabled(IsTransparent);

		In->ApplyToPass(Pass, With, At, Bounds, UsedMaterials[i], Stencil);

		Pass->DrawVertexBuffer(ModelVertexBuffers[i]);
	}
}

void engine::graphics::Model::DrawInstanced(Renderer* Render, GraphicsScene* In,
	const Transform& At, Camera* With, std::vector<Material*>& UsedMaterials,
	const BoundingBox& Bounds, bool Stencil, bool IsTransparent, size_t Count)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		auto Pass = Render->StartRender();
		if (IsTransparent != UsedMaterials[i]->IsTransparent)
		{
			continue;
		}
		Pass->SetBlendEnabled(IsTransparent);

		In->ApplyToPass(Pass, With, At, Bounds, UsedMaterials[i], Stencil);

		Pass->DrawVertexBufferInstanced(ModelVertexBuffers[i], Count);
	}
}

void engine::graphics::Model::SimpleDraw(Renderer* Render, const Transform& At, ShaderObject* Shader,
	std::vector<Material*>& UsedMaterials)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		if (UsedMaterials[i]->IsTransparent)
		{
			continue;
		}

		auto Pass = Render->StartRender();
		UsedMaterials[i]->ApplySimple(Pass, Shader);
		Shader->SetMatrix(Shader->ModelUniform, At.Matrix);
		Pass->DrawVertexBuffer(ModelVertexBuffers[i]);
	}
}

void engine::graphics::Model::SimpleDrawInstanced(Renderer* Render, const Transform& At, ShaderObject* Shader, std::vector<Material*>& UsedMaterials, size_t Count)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		if (UsedMaterials[i]->IsTransparent)
		{
			continue;
		}

		auto Pass = Render->StartRender();
		UsedMaterials[i]->ApplySimple(Pass, Shader);
		Shader->SetMatrix(Shader->ModelUniform, At.Matrix);
		Pass->DrawVertexBufferInstanced(ModelVertexBuffers[i], Count);
	}
}

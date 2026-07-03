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

		UsedMaterials[i]->Apply(Pass);
		ShaderObject* Used = UsedMaterials[i]->Shader;
		Used->Bind();

		if (Used == nullptr)
			continue;

		if (!Used->Unlit && In)
		{
			In->Lights.ApplyToShader(Used, Bounds);
			In->Shadows.BindUniforms(Pass, Used);
		}
		if (In)
		{
			With->UsedEnvironment->ApplyTo(Used);
		}

		if (!In)
			Pass->SetStencilValue(Stencil, i + 2);
		else
			Pass->SetStencilValue(Stencil, 1);

		// TODO: Replace camera params with Uniform buffers
		Used->SetMatrix(Used->ModelUniform, At.Matrix);
		Used->SetMatrix(Used->GetUniformLocation("u_view"), With->View);
		Used->SetMatrix(Used->GetUniformLocation("u_projection"), With->Projection);
		Used->SetVec3(Used->GetUniformLocation("u_cameraPos"), With->GetPosition());

		Pass->DrawVertexBuffer(ModelVertexBuffers[i]);
	}
}

void engine::graphics::Model::SimpleDraw(Renderer* Render, const Transform& At, ShaderObject* Shader,
	std::vector<Material*>& UsedMaterials)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		auto Pass = Render->StartRender();
		UsedMaterials[i]->ApplySimple(Pass, Shader);
		Shader->SetMatrix(Shader->ModelUniform, At.Matrix);
		Pass->DrawVertexBuffer(ModelVertexBuffers[i]);
	}
}

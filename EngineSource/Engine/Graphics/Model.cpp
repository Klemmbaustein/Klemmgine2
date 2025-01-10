#include "Model.h"
#include <Engine/Internal/OpenGL.h>
#include <Engine/File/ModelData.h>
#include <Engine/Scene.h>

engine::graphics::Model::Model(const ModelData* From)
{
	for (auto& i : From->Meshes)
	{
		ModelVertexBuffers.push_back(new VertexBuffer(i.Vertices, i.Indices));
	}
}

engine::graphics::Model::~Model()
{
	for (VertexBuffer* i : ModelVertexBuffers)
	{
		delete i;
	}
}

void engine::graphics::Model::Draw(const Transform& At, graphics::Camera* With, std::vector<Material*>& UsedMaterials)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		UsedMaterials[i]->Apply();
		ShaderObject* Used = UsedMaterials[i]->Shader;

		Scene::GetMain()->Shadows.BindUniforms(Used);

		if (Used == nullptr)
			continue;

		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_model"), 1, false, &At.Matrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_view"), 1, false, &With->View[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_projection"), 1, false, &With->Projection[0][0]);
		
		ModelVertexBuffers[i]->Draw();
	}
}

void engine::graphics::Model::SimpleDraw(const Transform& At, ShaderObject* Shader)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		glUniformMatrix4fv(glGetUniformLocation(Shader->ShaderID, "u_model"), 1, false, &At.Matrix[0][0]);
		ModelVertexBuffers[i]->Draw();
	}
}

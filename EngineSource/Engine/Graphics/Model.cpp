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

void engine::graphics::Model::Draw(Scene* In, const Transform& At, graphics::Camera* With, std::vector<Material*>& UsedMaterials)
{
	if (!In)
	{
		glStencilMask(0xFF);
	}

	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		UsedMaterials[i]->Apply();
		ShaderObject* Used = UsedMaterials[i]->Shader;

		if (In)
			In->Shadows.BindUniforms(Used);
		else
			glStencilFunc(GL_ALWAYS, GLint(i) + 2, 0xFF);

		if (Used == nullptr)
			continue;

		glUniformMatrix4fv(Used->ModelUniform, 1, false, &At.Matrix[0][0]);
		glUniformMatrix4fv(Used->GetUniformLocation("u_view"), 1, false, &With->View[0][0]);
		glUniformMatrix4fv(Used->GetUniformLocation("u_projection"), 1, false, &With->Projection[0][0]);
		Used->SetVec3(Used->GetUniformLocation("u_cameraPos"), With->GetPosition());

		ModelVertexBuffers[i]->Draw();
	}
	if (!In)
	{
		glStencilMask(0);
		glStencilFunc(GL_GEQUAL, 1, 0xFF);
	}
}

void engine::graphics::Model::SimpleDraw(const Transform& At, ShaderObject* Shader, std::vector<Material*>& UsedMaterials)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		UsedMaterials[i]->ApplySimple(Shader);
		glUniformMatrix4fv(Shader->ModelUniform, 1, false, &At.Matrix[0][0]);
		ModelVertexBuffers[i]->Draw();
	}
}

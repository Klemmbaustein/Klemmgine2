#include "Model.h"
#include <Engine/Internal/OpenGL.h>
#include <kui/Resource.h>
#include <Engine/File/ModelData.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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

void engine::graphics::Model::Draw(Vector3 At, graphics::Camera* With, std::vector<Material*>& UsedMaterials)
{
	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		UsedMaterials[i]->Apply();
		glm::mat4 ModelMatrix = glm::mat4(1);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(At.X, At.Y, At.Z));

		ShaderObject* Used = UsedMaterials[i]->Shader;

		if (Used == nullptr)
			continue;

		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_model"), 1, false, &ModelMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_view"), 1, false, &With->View[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(Used->ShaderID, "u_projection"), 1, false, &With->Projection[0][0]);
		
		ModelVertexBuffers[i]->Draw();
	}
}

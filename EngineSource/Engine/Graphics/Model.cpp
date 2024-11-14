#include "Model.h"
#include <GL/glew.h>
#include <kui/Resource.h>
#include <Engine/File/ModelData.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

engine::graphics::Model::Model(const ModelData* From)
{
	for (auto& i : From->Meshes)
	{
		ModelVertexBuffer.push_back(new VertexBuffer(i.Vertices, i.Indices));
	}
}

engine::graphics::Model::~Model()
{
}

void engine::graphics::Model::Draw(Vector3 At, graphics::Camera* With, ShaderObject* UsedShader)
{
	UsedShader->Bind();
	glm::mat4 ModelMatrix = glm::mat4(1);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(At.X, At.Y, At.Z));

	glUniformMatrix4fv(glGetUniformLocation(UsedShader->ShaderID, "u_model"), 1, false, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(UsedShader->ShaderID, "u_view"), 1, false, &With->View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(UsedShader->ShaderID, "u_projection"), 1, false, &With->Projection[0][0]);
	for (auto* i : ModelVertexBuffer)
	{
		i->Draw();
	}
}

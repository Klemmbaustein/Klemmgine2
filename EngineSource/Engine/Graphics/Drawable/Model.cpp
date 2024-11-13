#include "Model.h"
#include <GL/glew.h>
#include <kui/Resource.h>

engine::graphics::Model::Model(ModelData* From)
{
	for (auto& i : From->Meshes)
	{
		ModelVertexBuffer.push_back(new VertexBuffer(i.Vertices, i.Indices));
	}
	ModelShader = new ShaderObject(
		{ kui::resource::GetStringFile("res:shader/basic.vert") },
		{ kui::resource::GetStringFile("res:shader/basic.frag") }
	);

}

engine::graphics::Model::~Model()
{
	delete ModelShader;
}

void engine::graphics::Model::Draw(Camera* With)
{
	ModelShader->Bind();
	glm::mat4 ModelMatrix = glm::mat4(1);

	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_model"), 1, false, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_view"), 1, false, &With->View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_projection"), 1, false, &With->Projection[0][0]);
	for (auto* i : ModelVertexBuffer)
	{
		i->Draw();
	}
}

#include "Model.h"
#include <GL/glew.h>
#include <kui/Resource.h>
#include <glm/ext/matrix_transform.hpp>

engine::graphics::Model::Model()
{
	ModelVertexBuffer = new VertexBuffer({
		Vertex(Vector3(0.5f,  -0.5f, 0.0f)),
		Vertex(Vector3(0.5f, 0.5f, 0.0f)),
		Vertex(Vector3(-0.5f, -0.5f, 0.0f)),
		Vertex(Vector3(-0.5f,   0.5f, 0.0f)),
		}, { 0, 1, 2, 1, 3, 2 });

	ModelShader = new ShaderObject(
		{ kui::resource::GetStringFile("res:shader/basic.vert") },
		{ kui::resource::GetStringFile("res:shader/basic.frag") }
	);
}

engine::graphics::Model::~Model()
{
}

void engine::graphics::Model::Draw(Camera* With)
{
	ModelShader->Bind();
	glm::mat4 ModelMatrix = glm::mat4(1);

	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_model"), 1, false, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_view"), 1, false, &With->View[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ModelShader->ShaderID, "u_projection"), 1, false, &With->Projection[0][0]);
	ModelVertexBuffer->Draw();
}

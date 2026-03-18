#include "Model.h"
#include <Engine/Internal/OpenGL.h>
#include <Engine/File/ModelData.h>
#include <Engine/Scene.h>
#include <Engine/Input.h>

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

void engine::graphics::Model::Draw(GraphicsScene* In, const Transform& At, graphics::Camera* With,
	std::vector<Material*>& UsedMaterials, const BoundingBox& Bounds, bool Stencil)
{
	if (!In && Stencil)
	{
		glStencilMask(0xFF);
	}

	for (size_t i = 0; i < ModelVertexBuffers.size(); i++)
	{
		UsedMaterials[i]->Apply();
		ShaderObject* Used = UsedMaterials[i]->Shader;

		if (Used == nullptr)
			continue;

		if (!Used->Unlit && In)
		{
			auto Lights = In->Lights.GetLights(Bounds);

			for (size_t light = 0; light < 8; light++)
			{
				bool Active = light < Lights.size();
				// Actually creating a new string for each light slot for each drawable object ends up being a pretty big performance hit
				// Do this instead to speed things up a bit
				static const char* lights[8] = {
					"u_lights[0].isActive",
					"u_lights[1].isActive",
					"u_lights[2].isActive",
					"u_lights[3].isActive",
					"u_lights[4].isActive",
					"u_lights[5].isActive",
					"u_lights[6].isActive",
					"u_lights[7].isActive",
				};

				Used->SetInt(Used->GetUniformLocation(lights[light]), Active);

				if (Active)
				{
					Used->SetVec3(Used->GetUniformLocation(str::Format("u_lights[%i].position", light)), Lights[light]->Position);
				}
			}

			With->UsedEnvironment->ApplyTo(Used);
			In->Shadows.BindUniforms(Used);
		}

		if (!In)
			glStencilFunc(GL_ALWAYS, GLint(i) + 2, 0xFF);

		glUniformMatrix4fv(Used->ModelUniform, 1, false, &At.Matrix[0][0]);
		glUniformMatrix4fv(Used->GetUniformLocation("u_view"), 1, false, &With->View[0][0]);
		glUniformMatrix4fv(Used->GetUniformLocation("u_projection"), 1, false, &With->Projection[0][0]);
		Used->SetVec3(Used->GetUniformLocation("u_cameraPos"), With->GetPosition());

		ModelVertexBuffers[i]->Draw();
	}
	if (!In && Stencil)
	{
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

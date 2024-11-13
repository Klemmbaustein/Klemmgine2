#pragma once
#include <Engine/Types.h>
#include <vector>
#include <Engine/Graphics/Vertex.h>
#include <Engine/File/SerializedData.h>

namespace engine
{
	using namespace engine::graphics;
	
	struct ModelData : ISerializable
	{
		struct Mesh : ISerializable
		{
			Mesh();
			Mesh(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);

			std::vector<Vertex> Vertices;
			std::vector<uint32> Indices;

			virtual SerializedValue Serialize() override;
			virtual void DeSerialize(SerializedValue* From) override;
		};
		ModelData(string FilePath);
		ModelData();

		void ToFile(string FilePath);

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		std::vector<Mesh> Meshes;
	};
}
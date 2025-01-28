#pragma once
#include <Engine/Types.h>
#include <vector>
#include <Engine/Graphics/Vertex.h>
#include <Engine/File/SerializedData.h>
#include <Engine/Graphics/Model.h>
#include <unordered_map>
#include "AssetRef.h"

namespace engine
{	
	struct ModelData : ISerializable
	{
		struct Mesh : ISerializable
		{
			Mesh();
			Mesh(const std::vector<graphics::Vertex>& Vertices, const std::vector<uint32>& Indices);

			std::vector<graphics::Vertex> Vertices;
			std::vector<uint32> Indices;

			virtual SerializedValue Serialize() override;
			virtual void DeSerialize(SerializedValue* From) override;

			string Material;
		};
		ModelData(string FilePath);
		ModelData();
		~ModelData();

		void ToFile(string FilePath);

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		std::vector<Mesh> Meshes;
	};

	struct GraphicsModel
	{
		ModelData* Data = nullptr;
		graphics::Model* Drawable = nullptr;
		size_t References;
		std::map<void*, std::function<void()>> OnDereferenced;

		static void RegisterModel(const ModelData& Mesh, string Name, bool Lock = true);
		static void RegisterModel(AssetRef Asset, bool Lock = true);
		static GraphicsModel* GetModel(AssetRef Asset);
		static void ReferenceModel(GraphicsModel* Target);
		static void UnloadModel(GraphicsModel* Target);
		static void UnloadModel(AssetRef Asset);
	private:
		static std::unordered_map<string, GraphicsModel> Models;
	};
}
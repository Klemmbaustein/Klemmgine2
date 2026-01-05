#pragma once
#include <Core/Types.h>
#include <Core/Event.h>
#include <vector>
#include <Engine/Graphics/Vertex.h>
#include <Engine/Graphics/BoundingBox.h>
#include <Core/File/SerializedData.h>
#include <Engine/Graphics/Model.h>
#include <unordered_map>
#include "AssetRef.h"
#include <Core/File/BinaryStream.h>

namespace engine
{
	class Scene;

	struct ModelData : ISerializable
	{
		struct Mesh : ISerializable
		{
			Mesh(bool LoadMaterials)
			{
				this->LoadMaterials = LoadMaterials;
			}
			Mesh(const std::vector<graphics::Vertex>& Vertices, const std::vector<uint32>& Indices);

			std::vector<graphics::Vertex> Vertices;
			std::vector<uint32> Indices;
			graphics::BoundingBox Bounds;

			virtual SerializedValue Serialize() override;
			virtual void DeSerialize(SerializedValue* From) override;

			bool LoadMaterials = true;
			string Material;
		};
		ModelData(string FilePath, bool LoadMaterials);
		ModelData();
		~ModelData();

		void PreLoadMaterials(Scene* With);

		void ToFile(string FilePath);
		void ToBinary(IBinaryStream* To);

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		graphics::BoundingBox Bounds;
		std::vector<Mesh> Meshes;
		bool HasCollision = true;
		bool CastShadow = true;
		bool LoadMaterials = true;
	};

	struct GraphicsModel
	{
		ModelData* Data = nullptr;
		graphics::Model* Drawable = nullptr;
		size_t References = 0;
		Event<> OnDereferenced;
		Event<> OnMaterialsChanged;

		static GraphicsModel* RegisterModel(const ModelData& Mesh, string Name, bool Lock = true);
		static GraphicsModel* RegisterModel(AssetRef Asset, bool LoadMaterials = true, bool Lock = true);
		static GraphicsModel* GetModel(AssetRef Asset, bool LoadMaterials = true);
		static void ReferenceModel(GraphicsModel* Target);
		static void UnloadModel(GraphicsModel* Target);
		static void UnloadModel(AssetRef Asset);

		static GraphicsModel* UnitCube();
		static GraphicsModel* UnitPlane();

	private:
		static std::unordered_map<string, GraphicsModel> Models;
	};
}
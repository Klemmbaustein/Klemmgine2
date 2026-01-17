#pragma once
#include <Core/File/SerializedData.h>
#include "Texture.h"

namespace engine::graphics
{
	class ShaderObject;
	class Material : public ISerializable
	{
	public:

		Material(AssetRef File);

		static Material* MakeDefault();

		Material();
		Material(const Material&) = delete;
		~Material();

		Material& operator=(const Material&) = delete;

		struct MatTexture
		{
			string Name;
			TextureOptions Options;
		};

		struct Field
		{
			Field()
			{
				this->Int = 0;
			}
			enum class Type
			{
				None,
				Int,
				Float,
				Vec3,
				Texture,
				Bool,
			};

			string Name;
			uint32 UniformLocation = 0;

			union
			{
				int Int;
				float Float;
				Vector3 Vec3;
				struct
				{
					MatTexture* Name;
					const Texture* Value;
				} TextureValue;
			};
			Type FieldType = Type::None;
		};

		string VertexShader, FragmentShader;
		std::vector<Field> Fields;
		ShaderObject* Shader = nullptr;

		void ToFile(string Path);
		void ToStream(std::ostream& Stream);

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		void LoadTexture(Field& From);

		void Clear();
		void Apply();
		void ApplySimple(graphics::ShaderObject* With);
		void VerifyUniforms();
		void UpdateShader();
		Material::Field* FindField(string Name, Field::Type Type);
		bool UseTexture = false;
	private:
		size_t TextureField = SIZE_MAX;
		bool IsDefault = false;
		void SetToDefault();
	};
}
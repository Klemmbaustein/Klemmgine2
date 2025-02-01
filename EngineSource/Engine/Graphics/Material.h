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

		struct Field
		{
			Field()
			{

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
					string* Name;
					const Texture* Value;
				} TextureValue;
			};
			Type FieldType = Type::None;
		};

		string VertexShader, FragmentShader;
		std::vector<Field> Fields;
		ShaderObject* Shader = nullptr;

		void ToFile(string Path);

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		void Clear();
		void Apply();
		void VerifyUniforms();
		Material::Field* FindField(string Name, Field::Type Type);
	private:
		bool IsDefault = false;
		void SetToDefault();
		void UpdateShader();
	};
}
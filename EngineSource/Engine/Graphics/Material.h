#pragma once
#include <Engine/File/SerializedData.h>

namespace engine::graphics
{
	class ShaderObject;
	class Material : public ISerializable
	{
	public:

		Material(string FilePath);
		Material();
		~Material();

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
					uint32 Value;
				} Texture;
			};
			Type FieldType = Type::None;
		};

		string VertexShader, FragmentShader;
		std::vector<Field> Fields;
		ShaderObject* Shader = nullptr;

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		void Clear();
		void Apply();
	};
}
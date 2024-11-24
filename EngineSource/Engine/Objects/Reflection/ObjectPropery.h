#pragma once
#include <Engine/Types.h>
#include <Engine/File/SerializedData.h>

namespace engine
{
	class SceneObject;

	struct ObjPropertyBase : ISerializable
	{
		string Name;
	};

	struct AssetRef
	{
		string FilePath;
	};

	template<typename T>
	struct ObjProperty : public ObjPropertyBase
	{
	public:
		ObjProperty()
		{

		}

		ObjProperty(T Value, SceneObject* Obj)
		{
			this->Value = Value;
		}

		T Value;
	};

	template<>
	struct ObjProperty<string> : public ObjPropertyBase
	{
		ObjProperty(string Name, string Value, SceneObject* Obj);

		ObjProperty<string>& operator=(const string& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		string Value;
	};

	template<>
	struct ObjProperty<engine::AssetRef> : public ObjPropertyBase
	{
		ObjProperty(string Name, AssetRef Value, SceneObject* Obj);

		ObjProperty<AssetRef>& operator=(const AssetRef& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		AssetRef Value;
	};
}
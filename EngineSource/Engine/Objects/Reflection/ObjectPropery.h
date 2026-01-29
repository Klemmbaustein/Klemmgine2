#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>
#include "Engine/File/AssetRef.h"
#include <functional>

namespace engine
{
	class SceneObject;

	/**
	* @brief
	* A type that a reflected object property can have.
	*
	* Used by ObjProperty to define it's type.
	*/
	enum class PropertyType
	{
		/// A float property.
		Float,
		Int,
		/**
		* A string property.
		*
		* @see engine::string
		*/
		String,
		/**
		* An asset reference property.
		*
		* @see engine::AssetRef
		*/
		AssetRef,
		Bool,
		Vector3,
		/// Unknown property. Unknown properties can still be serialized and de-serialized, but can't be shown in the editor.
		Unknown,
	};

	struct ObjPropertyBase : ISerializable
	{
		PropertyType Type = PropertyType::Unknown;
		std::function<void()> OnChanged;
		string Name;
		bool IsHidden = false;

	protected:
		void RegisterSelf(SceneObject* Parent);
	};

	/**
	* @brief
	* An object property with an unknown serializable type.
	*
	* T must derive from ISerializable.
	*
	* @see ISerializable
	*/
	template<typename T>
	struct ObjProperty : public ObjPropertyBase
	{
	public:
		ObjProperty()
		{
		}

		ObjProperty(string Name, T Value, SceneObject* Obj)
		{
			this->Name = Name;
			this->Value = Value;
			RegisterSelf(Obj);
		}

		T Value;
	};

	template<>
	struct ObjProperty<engine::string> : public ObjPropertyBase
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
	struct ObjProperty<Vector3> : public ObjPropertyBase
	{
		ObjProperty(string Name, Vector3 Value, SceneObject* Obj);

		ObjProperty<Vector3>& operator=(const Vector3& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		Vector3 Value;
	};

	template<>
	struct ObjProperty<float> : public ObjPropertyBase
	{
		ObjProperty(string Name, float Value, SceneObject* Obj);

		ObjProperty<float>& operator=(const float& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		float Value;
	};

	template<>
	struct ObjProperty<int32> : public ObjPropertyBase
	{
		ObjProperty(string Name, int32 Value, SceneObject* Obj);

		ObjProperty<int32>& operator=(const int32& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		int32 Value;
	};

	template<>
	struct ObjProperty<bool> : public ObjPropertyBase
	{
		ObjProperty(string Name, bool Value, SceneObject* Obj);

		ObjProperty<bool>& operator=(const bool& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		bool Value;
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
#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>
#include "Engine/File/AssetRef.h"
#include <functional>
#include <memory>

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
		Array,
		/// Unknown property. Unknown properties can still be serialized and de-serialized, but can't be shown in the editor.
		Unknown,
	};

	enum class PropertyHint
	{
		None = 0,
		Vec3Color = 1 << 0,
		Vec3Rotation = 1 << 1,
		AssetEmptyIsDefault = 1 << 2
	};

	struct ObjPropertyBase : ISerializable
	{
		virtual ~ObjPropertyBase() = default;

		PropertyType Type = PropertyType::Unknown;
		PropertyHint Hints = PropertyHint::None;
		std::function<void()> OnChanged;
		string Name;
		bool IsHidden = false;

		bool HasHint(PropertyHint ToTest) const
		{
			return int(ToTest) & int(Hints);
		}

		void AddHint(PropertyHint NewHint)
		{
			Hints = PropertyHint(int(Hints) | int(NewHint));
		}

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
		ObjProperty() = default;

		ObjProperty<string>& operator=(const string& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		string Value;
	};

	struct ObjPropertyArrayBase : public ObjPropertyBase
	{
		std::vector<std::shared_ptr<ObjPropertyBase>> Values;
	};

	template<typename T2>
	struct ObjPropertyArray : public ObjPropertyArrayBase
	{
		//using ValArray = PropertyArray<T2>;

		ObjPropertyArray(string Name, SceneObject* Obj)
		{
			this->Type = PropertyType::Array;
			this->Name = Name;
			RegisterSelf(Obj);
		}
		ObjPropertyArray() = default;

		//ObjPropertyArray& operator=(const std::vector<std::unique_ptr<ObjPropertyBase>> Values)
		//{
		//	this->Values = Values;
		//	return *this;
		//}

		SerializedValue Serialize() override
		{
			SerializedValue v = std::vector<SerializedValue>{};

			for (auto& i : this->Values)
			{
				v.Append(i->Serialize());
			}

			return v;
		}

		void DeSerialize(SerializedValue* From) override
		{
			Values.clear();

			for (auto& i : From->GetArray())
			{
				std::shared_ptr<ObjPropertyBase> v = std::make_shared<ObjProperty<T2>>("_", T2{}, nullptr);
				v->OnChanged = [this] {
					this->OnChanged();
				};
				v->DeSerialize(&i);

				this->Values.push_back(v);
			}
		}

		ObjProperty<T2>* GetValue(size_t Index)
		{
			return static_cast<ObjProperty<T2>*>(this->Values[Index].get());
		}

		void Append(const T2& Item)
		{
			std::shared_ptr<ObjPropertyBase> v = std::make_shared<ObjProperty<T2>>("_", v, nullptr);
			v->OnChanged = [this] {
				this->OnChanged();
			};
			this->Values.push_back(v);
		}
	};

	template<>
	struct ObjProperty<Vector3> : public ObjPropertyBase
	{
		ObjProperty(string Name, Vector3 Value, SceneObject* Obj);
		ObjProperty() = default;

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
		ObjProperty() = default;

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
		ObjProperty() = default;

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
		ObjProperty() = default;

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
		ObjProperty() = default;

		ObjProperty<AssetRef>& operator=(const AssetRef& Target)
		{
			this->Value = Target;
			return *this;
		}

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		AssetRef Value;
	};

#define PROPERTY(t, n, eq, v) ObjProperty<t> n eq ObjProperty<t>(str::AddSpacesToName(# n), v, this)
}
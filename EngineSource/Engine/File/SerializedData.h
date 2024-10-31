#pragma once
#include <vector>
#include <Engine/Types.h>
#include <Engine/Vector.h>

namespace engine
{
	struct BinaryStream
	{

	};

	class SerializedData
	{
	public:
		enum class DataType : uint8
		{
			None,
			Int32,
			Byte,
			Boolean,
			Float,
			Vector3,
			String,
			Array,
			Object
		};

		class DataValue
		{
			union
			{
				int32 Int;
				uByte Byte;
				float Float;
				Vector3 Vec;
				string* String;
				std::vector<DataValue>* Array;
				std::vector<SerializedData>* Object;
			};
			DataType Type = DataType::None;

			void CopyFrom(const DataValue& From);
			void Free() const;

		public:

			DataType GetType() const;

			DataValue()
			{
				Int = 0;
			}

			DataValue(int32 Value)
			{
				Int = Value;
				Type = DataType::Int32;
			}
			DataValue(uByte Value)
			{
				Byte = Value;
				Type = DataType::Byte;
			}
			explicit DataValue(bool Value)
			{
				Byte = Value;
				Type = DataType::Boolean;
			}
			DataValue(float Value)
			{
				Float = Value;
				Type = DataType::Float;
			}
			DataValue(Vector3 Value)
			{
				Vec = Value;
				Type = DataType::Vector3;
			}
			DataValue(string Value)
			{
				String = new std::string(Value);
				Type = DataType::String;
			}
			DataValue(const char* Value)
			{
				String = new std::string(Value);
				Type = DataType::String;
			}
			DataValue(const std::vector<DataValue>& Array)
			{
				this->Array = new std::vector<DataValue>(Array);
				Type = DataType::Array;
			}
			DataValue(const std::vector<SerializedData>& Object)
			{
				this->Object = new std::vector<SerializedData>(Object);
				Type = DataType::Object;
			}

			DataValue(const DataValue& Other);

			DataValue& operator=(const DataValue& Other);

			~DataValue()
			{
				Free();
			}

			int32 GetInt() const;
			uByte GetByte() const;
			bool GetBool() const;
			float GetFloat() const;
			Vector3 GetVector3() const;
			string GetString() const;
			std::vector<DataValue>& GetArray();
			const std::vector<DataValue>& GetArray() const;

			std::vector<SerializedData>& GetObject();
			const std::vector<SerializedData>& GetObject() const;

			string ToString(size_t Depth) const;
			DataValue& At(string Name);
			DataValue& At(size_t Index);
			void Append(const SerializedData& New);
			void Append(const DataValue& New);
		};

		SerializedData()
		{

		}

		SerializedData(string Name, const DataValue& Value)
		{
			this->Name = Name;
			this->Value = Value;
		}

		DataValue& At(string Name);
		DataValue& At(size_t Index);
		void Append(const SerializedData& New);
		void Append(const DataValue& New);
		size_t Size() const;

		string ToString(size_t Depth) const;

		DataValue Value;
		string Name;
	};

	using SerializedValue = SerializedData::DataValue;

	class ISerializable
	{
	public:
		virtual SerializedValue Serialize() = 0;
		virtual void DeSerialize(SerializedValue* From) = 0;
	};

}
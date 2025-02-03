#pragma once
#include <vector>
#include <Core/Types.h>
#include <Core/Vector.h>

/**
* @defgroup serialize Serialization
* 
* @brief
* Classes related to serialized data.
* 
* Part of the core library.
* 
* @ingroup engine-core
*/

namespace engine
{
	/**
	* @brief
	* Exception thrown when an error with serialized data occurs.
	* 
	* @ingroup serialize
	*/
	class SerializeException : std::exception
	{
		string ErrorMsg;
	public:
		/// Constructs the exception with the given message.
		SerializeException(string Msg);
		/// Gets the error message of this exception.
		const char* what() const noexcept override;
	};

	/**
	* @brief
	* Exception thrown for errors reading or parsing files containing serialized data.
	* 
	* @ingroup serialize
	*/
	class SerializeReadException : public SerializeException
	{
	public:
		/// Constructs the exception with the given message.
		SerializeReadException(string Msg);
	};

	/**
	* @brief
	* A Serialized data value containing a name and value.
	* 
	* Serialized data is mostly used to store information in files.
	* This class itself only holds the data in a format that can be serialized, but the serializing itself is done by
	* the BinarySerializer and TextSerializer classes.
	* 
	* @ingroup serialize
	*
	* @see BinarySerializer
	* @see TextSerializer
	* @see ISerializable
	* @see DataValue
	*/
	class SerializedData
	{
	public:
		/**
		* @brief
		* Enum for the type of a serialized data value.
		*/
		enum class DataType : uint8
		{
			/// No type. The associated value is empty.
			Null,
			/// Signed 32 bit integer. Standard C/C++ `int` type. @see int32
			Int32,
			/// Unsigned 8 bit integer.
			/// @see uByte
			Byte,
			/// Boolean value. C++ `bool` type.
			Boolean,
			/// 32 bit float value. C++ `float` type.
			Float,
			/// Stores X, Y and Z components. @see engine::Vector3
			Vector3,
			/// String. @see engine::string
			String,
			/// A C++ vector of SerializedValue objects.
			/// @see SerializedValue 
			/// @see SerializedData::DataValue
			Array,
			/// Used by the binary serializer to save an array where all elements have the same type.
			/// Do not use. @see engine::BinarySerializer
			Internal_BinaryTypedArray,
			/// A C++ vector of SerializedData objects. @see engine::SerializedData
			Object,
			/// Stores X and Y components. @see engine::Vector2
			Vector2,
		};

		/**
		* @brief
		* A serialized value.
		* 
		* This object stores a type and a value that can be serialized.
		* 
		* @see SerializedData
		*
		* @ingroup serialize
		*/
		class DataValue
		{
		private:
			// Doxygen lists these as public members even though they're not.
			/// @cond
			union
			{
				int32 Int;
				uByte Byte;
				float Float;
				Vector3 Vec;
				Vector2 Vec2;
				string* String;
				std::vector<DataValue>* Array;
				std::vector<SerializedData>* Object;
			};
			/// @endcond
			DataType Type = DataType::Null;

			void CopyFrom(const DataValue& From);
			void Free() const;

		public:

			/// Gets the type stored for this value object.
			DataType GetType() const;

			/// Empty constructor. Constructs a DataValue where the type is Null.
			DataValue()
			{
				Int = 0;
			}

			/// Constructs a data value from a 32 bit integer.
			DataValue(int32 Value)
			{
				Int = Value;
				Type = DataType::Int32;
			}
			/// Constructs a data value from an unsigned byte.
			DataValue(uByte Value)
			{
				Byte = Value;
				Type = DataType::Byte;
			}
			/// Constructs a data value from a boolean.
			DataValue(bool Value)
			{
				Byte = Value;
				Type = DataType::Boolean;
			}
			/// Constructs a data value from a 32 bit float.
			DataValue(float Value)
			{
				Float = Value;
				Type = DataType::Float;
			}
			/// Constructs a data value from a Vector3.
			DataValue(Vector3 Value)
			{
				Vec = Value;
				Type = DataType::Vector3;
			}
			/// Constructs a data value from a Vector2.
			explicit DataValue(Vector2 Value)
			{
				Vec2 = Value;
				Type = DataType::Vector2;
			}
			/// Constructs a data value from a string.
			DataValue(string Value)
			{
				String = new std::string(Value);
				Type = DataType::String;
			}
			/// Constructs a data value from a char pointer.
			DataValue(const char* Value)
			{
				String = new std::string(Value);
				Type = DataType::String;
			}
			/// Constructs a data value from a vector of other data values. It's type will be DataType::Array.
			DataValue(const std::vector<DataValue>& Array)
			{
				this->Array = new std::vector<DataValue>(Array);
				Type = DataType::Array;
			}
			/// Constructs a data value from a vector of objects. It's type will be DataType::Object.
			DataValue(const std::vector<SerializedData>& Object)
			{
				this->Object = new std::vector<SerializedData>(Object);
				Type = DataType::Object;
			}

			/// Copies the value of one data value to the other.
			DataValue(const DataValue& Other);

			/// Copies the value of one data value to the other.
			DataValue& operator=(const DataValue& Other);

			/// Destructs and frees this value.
			~DataValue()
			{
				Free();
			}

			/**
			* @brief
			* Gets this value as an int.
			* 
			* If the type of this value isn't DataType::Int, this returns 0.
			*/
			int32 GetInt() const;
			/**
			* @brief
			* Gets this value as a byte.
			*
			* If the type of this value isn't DataType::Byte, this returns 0.
			*/
			uByte GetByte() const;
			/**
			* @brief
			* Gets this value as a boolean.
			*
			* If the type of this value isn't DataType::Bool, this returns false.
			*/
			bool GetBool() const;
			/**
			* @brief
			* Gets this value as a float.
			*
			* If the type of this value isn't DataType::Float, this returns 0.
			*/
			float GetFloat() const;
			/**
			* @brief
			* Gets this value as a Vector3.
			*
			* If the type of this value isn't DataType::Vector3, this returns Vector3(0).
			*/
			Vector3 GetVector3() const;
			/**
			* @brief
			* Gets this value as a Vector2.
			*
			* If the type of this value isn't DataType::Vector2, this returns Vector2(0).
			*/
			Vector2 GetVector2() const;
			/**
			* @brief
			* Gets this value as a string.
			*
			* If the type of this value isn't DataType::String, this returns an empty string.
			*/
			string GetString() const;
			/**
			* @brief
			* Gets this value as an array.
			*
			* If the type of this value isn't DataType::Array, it throws a SerializeException.
			* 
			* @throw SerializeException
			*/
			std::vector<DataValue>& GetArray();
			/**
			* @brief
			* Gets this value as an array.
			*
			* If the type of this value isn't DataType::Array, it throws a SerializeException.
			*
			* @throw SerializeException
			*/
			const std::vector<DataValue>& GetArray() const;

			/**
			* @brief
			* Gets this value as an object.
			*
			* If the type of this value isn't DataType::Object, it throws a SerializeException.
			*
			* @throw SerializeException
			*/
			std::vector<SerializedData>& GetObject();

			/**
			* @brief
			* Gets this value as an object.
			*
			* If the type of this value isn't DataType::Object, it throws a SerializeException.
			*
			* @throw SerializeException
			*/
			const std::vector<SerializedData>& GetObject() const;

			/**
			* @brief
			* Converts this value to a string.
			* 
			* This format is not equivalent with the format used by engine::TextSerializer.
			* It cannot be de serialized.
			*
			* @see engine::TextSerializer
			*/
			string ToString(size_t Depth) const;
			/// Gets the SerializedData with the given name if this is an object.
			DataValue& At(string Name);
			/// Gets the SerializedData with the given index if this is an object or array.
			DataValue& At(size_t Index);

			bool IsNull() const;

			/**
			* @brief
			* Checks if this value contains a key with the given value.
			* 
			* This only works of the data value is an object.
			*/
			bool Contains(string Name);
			/**
			* @brief
			* Appends the SerializedData object to the current object.
			* 
			* This only works of the data value is an object.
			*/
			void Append(const SerializedData& New);
			/**
			* @brief
			* Appends the DataValue object to the current array.
			*
			* This only works of the data value is an array.
			*/
			void Append(const DataValue& New);
		};

		/**
		* @brief
		* Empty serialized data value.
		*/
		SerializedData()
		{

		}

		/**
		* @brief
		* Creates a serialized value with the name and value.
		*/
		SerializedData(string Name, const DataValue& Value)
		{
			this->Name = Name;
			this->Value = Value;
		}

		/// Gets the SerializedData with the given name if this is an object.
		DataValue& At(string Name);
		/// Gets the SerializedData with the given index if this is an object or array.
		DataValue& At(size_t Index);
		/**
		* @brief
		* Appends the SerializedData object to the current object.
		*
		* This only works of the data value is an object.
		*/
		void Append(const SerializedData& New);
		/**
		* @brief
		* Appends the DataValue object to the current array.
		*
		* This only works of the data value is an array.
		*/
		void Append(const DataValue& New);
		/**
		* @brief
		* Gets the size of this array or object.
		*/
		size_t Size() const;


		/**
		* @brief
		* Converts this SerializedData to a string.
		*
		* This format is not equivalent with the format used by engine::TextSerializer.
		* It cannot be de serialized.
		*
		* @see engine::TextSerializer
		* @see DataValue::ToString()
		*/
		string ToString(size_t Depth) const;

		/// The value of this data entry.
		DataValue Value;
		/// The name of this data entry.
		string Name;
	};

	/**
	* @brief
	* SerializedData::DataValue alias.
	* 
	* SerializedValue stores a type and value.
	* 
	* @ingroup serialize
	*/
	using SerializedValue = SerializedData::DataValue;

	/**
	* @brief
	* An interface for a serializable object.
	* 
	* A serializable object can save and load it's state from a engine::SerializedValue object.
	* In theory, any Serializable can be the value of an engine::ObjProperty.
	* 
	* @ingroup serialize
	*/
	class ISerializable
	{
	public:

		/**
		* @brief
		* Function to serialize this object to the SerializedValue structure.
		* 
		* Used often to save the state of an object implementing this interface to a file.
		*/
		virtual SerializedValue Serialize() = 0;
		/**
		* @brief
		* Function to de serialize this object from a SerializedValue structure.
		*
		* Used often to load the state of an object implementing this interface to a file.
		*/
		virtual void DeSerialize(SerializedValue* From) = 0;
	};

}
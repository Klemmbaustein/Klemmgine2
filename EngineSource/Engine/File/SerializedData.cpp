#include "SerializedData.h"

void engine::SerializedData::DataValue::CopyFrom(const DataValue& From)
{
	Free();
	Type = From.Type;

	switch (Type)
	{
	case engine::SerializedData::DataType::Int32:
		Int = From.Int;
		break;
	case engine::SerializedData::DataType::Byte:
	case engine::SerializedData::DataType::Boolean:
		Byte = From.Byte;
		break;
	case engine::SerializedData::DataType::Float:
		Float = From.Float;
		break;
	case engine::SerializedData::DataType::Vector2:
		Vec2 = From.Vec2;
		break;
	case engine::SerializedData::DataType::Vector3:
		Vec = From.Vec;
		break;
	case engine::SerializedData::DataType::String:
		String = new string(*From.String);
		break;
	case engine::SerializedData::DataType::Array:
		Array = new std::vector<DataValue>(*From.Array);
		break;
	case engine::SerializedData::DataType::Object:
		Object = new std::vector<SerializedData>(*From.Object);
		break;
	case engine::SerializedData::DataType::Null:
	default:
		Int = 0;
		break;
	}
}

void engine::SerializedData::DataValue::Free() const
{
	if (Type == DataType::Array || Type == DataType::Internal_BinaryTypedArray)
	{
		delete Array;
	}
	if (Type == DataType::Object)
	{
		delete Object;
	}
	if (Type == DataType::String)
	{
		delete String;
	}
}

engine::SerializedData::DataType engine::SerializedData::DataValue::GetType() const
{
	return Type;
}

engine::SerializedData::DataValue::DataValue(const DataValue& Other)
{
	CopyFrom(Other);
}

engine::SerializedData::DataValue& engine::SerializedData::DataValue::operator=(const DataValue& Other)
{
	CopyFrom(Other);
	return *this;
}

int32 engine::SerializedData::DataValue::GetInt() const
{
	if (Type == DataType::Int32)
		return Int;
	return 0;
}

uByte engine::SerializedData::DataValue::GetByte() const
{
	if (Type == DataType::Byte)
		return Byte;
	return 0;
}

bool engine::SerializedData::DataValue::GetBool() const
{
	if (Type == DataType::Boolean)
		return Byte;
	return false;
}

float engine::SerializedData::DataValue::GetFloat() const
{
	if (Type == DataType::Float)
		return Float;
	return 0.0f;
}

engine::Vector3 engine::SerializedData::DataValue::GetVector3() const
{
	if (Type == DataType::Vector3)
		return Vec;
	return Vector3();
}

engine::Vector2 engine::SerializedData::DataValue::GetVector2() const
{
	if (Type == DataType::Vector2)
		return Vec2;
	return Vector2();
}

engine::string engine::SerializedData::DataValue::GetString() const
{
	if (Type == DataType::String)
		return *String;
	return "";
}

std::vector<engine::SerializedData::DataValue>& engine::SerializedData::DataValue::GetArray()
{
	if (Type != DataType::Array)
		throw SerializeException("GetArray called on a value that is not an array value.");
	return *Array;
}

const std::vector<engine::SerializedData::DataValue>& engine::SerializedData::DataValue::GetArray() const
{
	if (Type != DataType::Array)
		throw SerializeException("GetArray called on a value that is not an array value.");
	return *Array;
}

std::vector<engine::SerializedData>& engine::SerializedData::DataValue::GetObject()
{
	if (Type != DataType::Object)
		throw SerializeException("GetObject called on a value that is not an object value.");
	return *Object;
}

const std::vector<engine::SerializedData>& engine::SerializedData::DataValue::GetObject() const
{
	if (Type != DataType::Object)
		throw SerializeException("GetObject called on a value that is not an object value.");
	return *Object;
}

engine::string engine::SerializedData::DataValue::ToString(size_t Depth) const
{
	switch (GetType())
	{
	case DataType::Int32:
		return std::to_string(GetInt());

	case DataType::Boolean:
		return GetBool() ? "true" : "false";

	case DataType::Byte:
		return std::to_string(GetByte());

	case DataType::String:
		return GetString();

	case DataType::Float:
		return std::to_string(GetFloat());

	case DataType::Vector3:
		return GetVector3().ToString();

	case DataType::Vector2:
		return GetVector2().ToString();

	case DataType::Array:
	{
		string Out = "[ ";
		for (auto& i : GetArray())
		{
			Out.append(i.ToString(Depth + 1) + " ");
		}
		Out.push_back(']');
		return Out;
	}
	case DataType::Object:
	{
		string Out = "{\n";
		for (auto& i : GetObject())
		{
			Out.append(i.ToString(Depth + 1) + " ");
		}
		for (size_t i = 0; i < Depth; i++)
		{
			Out.push_back('\t');
		}
		Out.pop_back();
		Out.push_back('}');
		return Out;
	}
	case DataType::Null:
	default:
		break;
	}
	return "";
}

engine::SerializedData::DataValue& engine::SerializedData::DataValue::At(string Name)
{
	auto& Object = GetObject();

	for (SerializedData& Element : Object)
	{
		if (Element.Name == Name)
		{
			return Element.Value;
		}
	}
	throw SerializeException("At(string Name): Given name does not exist in this object.");
}

engine::SerializedData::DataValue& engine::SerializedData::DataValue::At(size_t Index)
{
	if (GetType() == DataType::Object)
	{
		auto& Object = GetObject();

		return Object.at(Index).Value;
	}
	else if (GetType() == DataType::Array)
	{
		auto& Array = GetArray();

		return Array.at(Index);
	}
	throw SerializeException("Invalid type for SerializedValue::At(size_t Index). Has to be array or object.");
}

bool engine::SerializedData::DataValue::Contains(string Name)
{
	auto& Obj = GetObject();
	for (auto& i : Obj)
	{
		if (i.Name == Name)
			return true;
	}
	return false;
}

void engine::SerializedData::DataValue::Append(const SerializedData& New)
{
	auto& Object = GetObject();
	Object.push_back(New);
}

void engine::SerializedData::DataValue::Append(const DataValue& New)
{
	auto& Array = GetArray();
	Array.push_back(New);
}

engine::SerializedData::DataValue& engine::SerializedData::At(string Name)
{
	return Value.At(Name);
}

engine::SerializedData::DataValue& engine::SerializedData::At(size_t Index)
{
	return Value.At(Index);
}

void engine::SerializedData::Append(const SerializedData& New)
{
	Value.Append(New);
}

void engine::SerializedData::Append(const DataValue& New)
{
	Value.Append(New);
}

size_t engine::SerializedData::Size() const
{
	return Value.GetArray().size();
}

engine::string engine::SerializedData::ToString(size_t Depth) const
{
	string Out;
	for (size_t i = 0; i < Depth; i++)
	{
		Out.push_back('\t');
	}

	Out += Name + ": " + std::to_string(int32(Value.GetType())) + " = " + Value.ToString(Depth) + "\n";
	return Out;
}

engine::SerializeException::SerializeException(string Msg)
{
	this->ErrorMsg = Msg;
}

const char* engine::SerializeException::what() const noexcept
{
	return this->ErrorMsg.c_str();
}

engine::SerializeReadException::SerializeReadException(string Msg)
	: SerializeException(str::Format("File read error: %s", Msg.c_str()))
{
}

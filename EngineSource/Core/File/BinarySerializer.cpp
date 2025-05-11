#include "BinarySerializer.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <filesystem>
using namespace engine;

const string engine::BinarySerializer::FORMAT_VERSION = "0";
void BinarySerializer::ToBinaryData(const std::vector<SerializedData>& Target, IBinaryStream* Out, string FormatIdentifier)
{
	if (!FormatIdentifier.empty())
	{
		Out->WriteString(FormatIdentifier + "/" + FORMAT_VERSION);
	}
	uint32 Size = uint32(Target.size());
	Out->WriteValue(Size);
	for (SerializedData i : Target)
	{
		Out->WriteString(i.Name);
		ValueToBinaryData(i.Value, Out);
	}
}

void BinarySerializer::ValueToBinaryData(const SerializedValue& Target, IBinaryStream* Out, SerializedData::DataType Type)
{
	bool InitialTypeChanged = false;
	if (Type == SerializedData::DataType::Null)
	{
		InitialTypeChanged = true;
		Type = Target.GetType();
		if (Type != SerializedData::DataType::Array)
			Out->WriteByte(uByte(Type));
	}

	switch (Type)
	{
	case engine::SerializedData::DataType::Int32:
	{
		Out->WriteValue(Target.GetInt());
		break;
	}
	case engine::SerializedData::DataType::Byte:
		Out->WriteValue(uByte(Target.GetByte()));
		break;
	case engine::SerializedData::DataType::Boolean:
		Out->WriteByte(uByte(Target.GetBool()));
		break;
	case engine::SerializedData::DataType::Float:
	{
		Out->WriteValue(Target.GetFloat());
		break;
	}
	case engine::SerializedData::DataType::Vector3:
	{
		Out->WriteValue(Target.GetVector3());
		break;
	}
	case engine::SerializedData::DataType::Vector2:
	{
		Out->WriteValue(Target.GetVector2());
		break;
	}
	case engine::SerializedData::DataType::String:
		Out->WriteString(Target.GetString());
		break;
	case engine::SerializedData::DataType::Array:
	{
		auto& Array = Target.GetArray();

		bool SameType = true;
		SerializedData::DataType tp = SerializedData::DataType::Null;
		if (!InitialTypeChanged)
		{
			for (auto& i : Array)
			{
				if (i.GetType() == SerializedData::DataType::Null)
				{
					SameType = false;
					break;
				}
				else if (tp == SerializedData::DataType::Null)
				{
					tp = i.GetType();
				}
				else if (i.GetType() != tp)
				{
					SameType = false;
					break;
				}
			}
		}
		if (!SameType && !InitialTypeChanged)
		{
			uint32 Size = uint32(Target.GetArray().size());
			Out->WriteByte(uByte(Type));
			Out->WriteValue(Size);
			for (const SerializedValue& i : Target.GetArray())
			{
				ValueToBinaryData(i, Out);
			}
			break;
		}
		Out->WriteByte(uByte(SerializedData::DataType::Internal_BinaryTypedArray));
		[[fallthrough]];
	}
	case engine::SerializedData::DataType::Internal_BinaryTypedArray:
	{
		auto& Array = Target.GetArray();

		uint32 Size = uint32(Array.size());
		Out->WriteValue(Size);

		if (Size == 0)
		{
			auto ArrayType = SerializedData::DataType::Null;
			Out->WriteValue(ArrayType);
			break;
		}

		auto ArrayType = Array.at(0).GetType();
		Out->WriteValue(ArrayType);
		for (const SerializedValue& i : Array)
		{
			ValueToBinaryData(i, Out, ArrayType);
		}
		break;
	}
	case engine::SerializedData::DataType::Object:
		ToBinaryData(Target.GetObject(), Out, "");
		break;
	case engine::SerializedData::DataType::Null:
	default:
		break;
	}
}

void BinarySerializer::ToFile(const std::vector<SerializedData>& Target, string FileName, string FormatIdentifier)
{
	FileStream TargetFile = FileStream(FileName, false);

	ToBinaryData(Target, &TargetFile, FormatIdentifier);
}

std::vector<SerializedData> engine::BinarySerializer::FromFile(string File, string FormatIdentifier)
{
	std::vector<SerializedData> Out;
	FromFile(File, FormatIdentifier, Out);
	return Out;
}

std::vector<SerializedData> engine::BinarySerializer::FromStream(IBinaryStream* Stream, string FormatIdentifier)
{
	std::vector<SerializedData> Out;
	FromStream(Stream, Out, FormatIdentifier);
	return Out;
}

void engine::BinarySerializer::FromStream(IBinaryStream* Stream, std::vector<SerializedData>& To, string FormatIdentifier)
{
	if (Stream == nullptr)
	{
		throw SerializeReadException("Attempted to read binary data from an empty stream!");
	}

	string FormatName;
	Stream->ReadString(FormatName);
	string ExpectedFormat = FormatIdentifier + "/" + FORMAT_VERSION;

	if (FormatName.size() > 16)
	{
		throw SerializeReadException("Attempted to read binary file but it doesn't appear to have a valid format name.");
	}

	if (FormatName != ExpectedFormat)
	{
		throw SerializeReadException(str::Format("Attempted to read binary file with format '%s' but the file has format '%s'",
			ExpectedFormat.c_str(), FormatName.c_str()));
	}

	int32 Length = Stream->Get<int32>();
	for (int32 i = 0; i < Length; i++)
	{
		auto& New = To.emplace_back();
		ReadSerializedData(Stream, New);
	}

}

void engine::BinarySerializer::FromFile(string File, string FormatIdentifier, std::vector<SerializedData>& To)
{
	if (!std::filesystem::exists(File) || !std::filesystem::is_regular_file(File))
	{
		throw SerializeReadException(str::Format("Failed reading '%s'. File does not exist or is not a regular file.",
			File.c_str()));
	}

	FileStream Stream = FileStream(File, true);
	FromStream(&Stream, To, FormatIdentifier);
}

void engine::BinarySerializer::ReadSerializedData(IBinaryStream* From, SerializedData& Out)
{
	From->ReadString(Out.Name);
	ReadValue(From, Out.Value);
}

void engine::BinarySerializer::ReadValue(IBinaryStream* From, SerializedValue& To, SerializedData::DataType Type)
{
	if (Type == SerializedData::DataType::Null)
		Type = From->Get<SerializedData::DataType>();

	switch (Type)
	{
	case SerializedData::DataType::Int32:
		To = From->Get<int32>();
		break;

	case SerializedData::DataType::Byte:
		To = From->Get<uByte>();
		break;

	case SerializedData::DataType::Boolean:
		To = SerializedValue(bool(From->Get<uByte>()));
		break;

	case SerializedData::DataType::Float:
		To = From->Get<float>();
		break;

	case SerializedData::DataType::Vector2:
		To = SerializedValue(From->Get<Vector2>());
		break;

	case SerializedData::DataType::Vector3:
		To = From->Get<Vector3>();
		break;

	case SerializedData::DataType::String:
	{
		string String;
		From->ReadString(String);
		To = String;
		break;
	}
	case SerializedData::DataType::Array:
	{
		To = std::vector<SerializedValue>();
		int32 Length = From->Get<int32>();
		if (Length > UINT32_MAX)
			throw new SerializeReadException("Array too long");
		To.GetArray().reserve(Length);
		for (int32 i = 0; i < Length; i++)
		{
			ReadValue(From, To.GetArray().emplace_back());
		}
		break;
	}

	case SerializedData::DataType::Internal_BinaryTypedArray:
	{
		To = std::vector<SerializedValue>();
		int32 Length = From->Get<int32>();
		To.GetArray().reserve(Length);
		if (Length > UINT32_MAX)
			throw new SerializeReadException("Array too long");

		auto ArrayType = From->Get<SerializedData::DataType>();
		for (int32 i = 0; i < Length; i++)
		{
			ReadValue(From, To.GetArray().emplace_back(), ArrayType);
		}
		break;
	}

	case SerializedData::DataType::Object:
	{
		To = std::vector<SerializedData>();

		int32 Length = From->Get<int32>();

		if (Length > UINT32_MAX)
			throw new SerializeReadException("Object too long");

		To.GetObject().reserve(Length);
		for (int32 i = 0; i < Length; i++)
		{
			ReadSerializedData(From, To.GetObject().emplace_back());
		}
		break;
	}

	case SerializedData::DataType::Null:
	default:
		To = SerializedValue();
		break;
	}
}

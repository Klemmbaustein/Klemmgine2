#include "BinarySerializer.h"
#include <fstream>
#include <iostream>
#include <cstring>
using namespace engine;

const string engine::BinarySerializer::FormatVersion = "0";
void BinarySerializer::ToBinaryData(const std::vector<SerializedData>& Target, std::vector<uByte>& Out, string FormatIdentifier)
{
	if (!FormatIdentifier.empty())
	{
		WriteString(FormatIdentifier + "/" + FormatVersion, Out);
	}
	uint32 Size = uint32(Target.size());
	CopyTo(&Size, sizeof(Size), Out);
	for (SerializedData i : Target)
	{
		WriteString(i.Name, Out);
		ValueToBinaryData(i.Value, Out);
	}
}

void BinarySerializer::ValueToBinaryData(const SerializedValue& Target, std::vector<uByte>& Out)
{
	SerializedData::DataType Type = Target.GetType();
	Out.push_back(uByte(Type));

	switch (Type)
	{
	case engine::SerializedData::DataType::Int32:
	{
		int32 Value = Target.GetInt();
		CopyTo(&Value, sizeof(Value), Out);
		break;
	}
	case engine::SerializedData::DataType::Byte:
		Out.push_back(uByte(Target.GetByte()));
		break;
	case engine::SerializedData::DataType::Boolean:
		Out.push_back(uByte(Target.GetBool()));
		break;
	case engine::SerializedData::DataType::Float:
	{
		float Value = Target.GetFloat();
		CopyTo(&Value, sizeof(Value), Out);
		break;
	}
	case engine::SerializedData::DataType::Vector3:
	{
		Vector3 Value = Target.GetVector3();
		CopyTo(&Value, sizeof(Value), Out);
		break;
	}
	case engine::SerializedData::DataType::String:
		WriteString(Target.GetString(), Out);
		break;
	case engine::SerializedData::DataType::Array:
	{
		uint32 Size = uint32(Target.GetArray().size());
		CopyTo(&Size, sizeof(Size), Out);
		for (const SerializedValue& i : Target.GetArray())
		{
			ValueToBinaryData(i, Out);
		}
		break;
	}
	case engine::SerializedData::DataType::Object:
		ToBinaryData(Target.GetObject(), Out, "");
		break;
	case engine::SerializedData::DataType::None:
	default:
		break;
	}
}

void BinarySerializer::ToFile(const std::vector<SerializedData>& Target, string FileName, string FormatIdentifier)
{
	std::vector<uByte> Bytes;

	ToBinaryData(Target, Bytes, FormatIdentifier);

	std::ofstream Out = std::ofstream(FileName, std::ios::binary | std::ios::out);
	Out.write((char*)Bytes.data(), Bytes.size());
	Out.close();
}

std::vector<SerializedData> engine::BinarySerializer::FromFile(string File, string FormatIdentifier)
{
	std::streampos FileSize = 0;
	std::ifstream In = std::ifstream(File, std::ios::binary);

	FileSize = In.tellg();
	In.seekg(0, std::ios::end);
	FileSize = In.tellg() - FileSize;

	BinaryStream Stream;
	Stream.Bytes.resize(FileSize);
	In.seekg(0, std::ios::beg);
	In.read((char*)Stream.Bytes.data(), FileSize);
	In.close();

	string FormatName = ReadString(Stream);
	string ExpectedFormat = FormatIdentifier + "/" + FormatVersion;
	if (FormatName != ExpectedFormat)
	{
		std::cerr << "File read error: Attempted to read '" << File << "' with format '" << ExpectedFormat << "' but the file has format '" << FormatName << "'" << std::endl;
		return {};
	}

	int32 Length = Stream.Get<int32>();

	std::vector<SerializedData> Out;

	for (int32 i = 0; i < Length; i++)
	{
		Out.push_back(ReadSerializedData(Stream));
	}

	return Out;
}

SerializedData engine::BinarySerializer::ReadSerializedData(BinaryStream& From)
{
	string Name = ReadString(From);

	return SerializedData(Name, ReadValue(From));
}

SerializedValue engine::BinarySerializer::ReadValue(BinaryStream& From)
{
	auto Type = From.Get<SerializedData::DataType>();

	switch (Type)
	{
	case SerializedData::DataType::Int32:
		return From.Get<int32>();

	case SerializedData::DataType::Byte:
		return From.Get<uByte>();

	case SerializedData::DataType::Boolean:
		return SerializedValue(bool(From.Get<uByte>()));

	case SerializedData::DataType::Float:
		return From.Get<float>();

	case SerializedData::DataType::Vector3:
		return From.Get<Vector3>();

	case SerializedData::DataType::String:
		return ReadString(From);

	case SerializedData::DataType::Array:
	{
		std::vector<SerializedValue> Out;
		int32 Length = From.Get<int32>();
		for (int32 i = 0; i < Length; i++)
		{
			Out.push_back(ReadValue(From));
		}

		return Out;
	}

	case SerializedData::DataType::Object:
	{
		std::vector<SerializedData> Out;
		int32 Length = From.Get<int32>();
		for (int32 i = 0; i < Length; i++)
		{
			Out.push_back(ReadSerializedData(From));
		}

		return Out;
	}

	case SerializedData::DataType::None:
	default:
		return SerializedValue();
	}
}

void BinarySerializer::WriteString(string Str, std::vector<uByte>& Out)
{
	for (char i : Str)
	{
		if (!i)
			break;
		Out.push_back(uByte(i));
	}
	Out.push_back(0);
}

void engine::BinarySerializer::CopyTo(void* Data, size_t Size, std::vector<uByte>& Out)
{
	Out.resize(Out.size() + Size);
	uByte* First = &Out[Out.size() - Size];
	memcpy(First, Data, Size);
}

uByte engine::BinarySerializer::BinaryStream::GetByte()
{
	if (Empty())
		return 0;
	return Bytes[StreamPos++];
}

bool engine::BinarySerializer::BinaryStream::Empty() const
{
	return Bytes.size() <= StreamPos;
}

string engine::BinarySerializer::ReadString(BinaryStream& From)
{
	string Out;
	uByte Char;
	while (!From.Empty() && (Char = From.GetByte()))
	{
		Out.push_back(char(Char));
	}

	return Out;
}

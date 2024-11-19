#include "BinarySerializer.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
using namespace engine;

const string engine::BinarySerializer::FORMAT_VERSION = "0";
void BinarySerializer::ToBinaryData(const std::vector<SerializedData>& Target, std::vector<uByte>& Out, string FormatIdentifier)
{
	if (!FormatIdentifier.empty())
	{
		WriteString(FormatIdentifier + "/" + FORMAT_VERSION, Out);
	}
	uint32 Size = uint32(Target.size());
	CopyTo(&Size, sizeof(Size), Out);
	for (SerializedData i : Target)
	{
		WriteString(i.Name, Out);
		ValueToBinaryData(i.Value, Out);
	}
}

void BinarySerializer::ValueToBinaryData(const SerializedValue& Target, std::vector<uByte>& Out, SerializedData::DataType Type)
{
	bool InitialTypeChanged = false;
	if (Type == SerializedData::DataType::None)
	{
		InitialTypeChanged = true;
		Type = Target.GetType();
		Out.push_back(uByte(Type));
	}

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
		auto& Array = Target.GetArray();

		bool SameType = true;
		SerializedData::DataType tp = SerializedData::DataType::None;
		if (!InitialTypeChanged)
		{
			for (auto& i : Array)
			{
				if (i.GetType() == SerializedData::DataType::None)
				{
					SameType = false;
					break;
				}
				else if (tp == SerializedData::DataType::None)
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
			CopyTo(&Size, sizeof(Size), Out);
			for (const SerializedValue& i : Target.GetArray())
			{
				ValueToBinaryData(i, Out);
			}
			break;
		}
		Out[Out.size() - 1] = uByte(engine::SerializedData::DataType::BinaryTypedArray);
		[[fallthrough]];
	}
	case engine::SerializedData::DataType::BinaryTypedArray:
	{
		auto& Array = Target.GetArray();

		uint32 Size = uint32(Array.size());
		CopyTo(&Size, sizeof(Size), Out);

		if (Size == 0)
		{
			auto ArrayType = SerializedData::DataType::None;
			CopyTo(&ArrayType, sizeof(ArrayType), Out);
		}

		auto ArrayType = Array.at(0).GetType();
		CopyTo(&ArrayType, sizeof(ArrayType), Out);
		for (const SerializedValue& i : Array)
		{
			ValueToBinaryData(i, Out, ArrayType);
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
	string ExpectedFormat = FormatIdentifier + "/" + FORMAT_VERSION;
	if (FormatName != ExpectedFormat)
	{
		std::cerr << str::Format("Attempted to read binary file '%s' with format '%s' but the file has format '%s'",
			File.c_str(), ExpectedFormat.c_str(), FormatName.c_str());
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

	SerializedValue Out;
	ReadValue(From, Out);
	return SerializedData(Name, Out);
}

void engine::BinarySerializer::ReadValue(BinaryStream& From, SerializedValue& To, SerializedData::DataType Type)
{
	if (Type == SerializedData::DataType::None)
		Type = From.Get<SerializedData::DataType>();

	switch (Type)
	{
	case SerializedData::DataType::Int32:
		To = From.Get<int32>();
		break;

	case SerializedData::DataType::Byte:
		To = From.Get<uByte>();
		break;

	case SerializedData::DataType::Boolean:
		To = SerializedValue(bool(From.Get<uByte>()));
		break;

	case SerializedData::DataType::Float:
		To = From.Get<float>();
		break;

	case SerializedData::DataType::Vector3:
		To = From.Get<Vector3>();
		break;

	case SerializedData::DataType::String:
		To = ReadString(From);
		break;

	case SerializedData::DataType::Array:
	{
		To = std::vector<SerializedValue>();
		int32 Length = From.Get<int32>();
		To.GetArray().reserve(Length);
		for (int32 i = 0; i < Length; i++)
		{
			To.GetArray().push_back(SerializedData::DataValue());
			auto& Last = To.GetArray()[To.GetArray().size() - 1];
			ReadValue(From, Last);
		}
		break;
	}

	case SerializedData::DataType::BinaryTypedArray:
	{
		To = std::vector<SerializedValue>();
		int32 Length = From.Get<int32>();
		To.GetArray().reserve(Length);
		auto ArrayType = From.Get<SerializedData::DataType>();
		for (int32 i = 0; i < Length; i++)
		{
			To.GetArray().push_back(SerializedData::DataValue());
			auto& Last = To.GetArray()[To.GetArray().size() - 1];
			ReadValue(From, Last, ArrayType);
		}
		break;
	}

	case SerializedData::DataType::Object:
	{
		To = std::vector<SerializedData>();

		int32 Length = From.Get<int32>();
		To.GetObject().reserve(Length);
		for (int32 i = 0; i < Length; i++)
		{
			To.GetObject().push_back(ReadSerializedData(From));
		}
		break;
	}

	case SerializedData::DataType::None:
	default:
		To = SerializedValue();
		break;
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

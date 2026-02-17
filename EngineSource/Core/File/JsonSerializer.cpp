#include "JsonSerializer.h"
#include <fstream>
#include <filesystem>

using namespace engine;

void engine::JsonSerializer::ToStream(const SerializedValue& Target,
	std::ostream& Stream, WriteOptions Opt)
{
	ValueToString(Target, Stream, 0, Opt);
}

void engine::JsonSerializer::ToFile(const SerializedValue& Target, string File, WriteOptions Opt)
{
	std::ofstream Out = std::ofstream(File);
	ToStream(Target, Out, Opt);
	Out.close();
}

void engine::JsonSerializer::WriteObject(const std::vector<SerializedData>& Target, std::ostream& Stream, uint32 Depth, WriteOptions Opt)
{
	Stream << "{";

	if (Opt.Indent)
	{
		Stream << "\n";
	}

	for (auto i = Target.begin(); i < Target.end(); i++)
	{
		if (Opt.Indent)
		{
			WriteIndent(Depth + 1, Stream);
		}
		Stream << "\"" << EscapeString(i->Name) << "\"";
		Stream << (Opt.Indent ? ": " : ":");
		ValueToString(i->Value, Stream, Depth + 1, Opt);

		if (i != Target.end() - 1)
		{
			Stream << ",";
		}
		if (Opt.Indent)
		{
			Stream << "\n";
		}
	}
	if (Opt.Indent)
	{
		WriteIndent(Depth, Stream);
	}
	Stream << "}";
}

void engine::JsonSerializer::WriteIndent(uint32 Depth, std::ostream& Stream)
{
	for (uint32 i = 0; i < Depth; i++)
	{
		Stream << "\t";
	}
}

void engine::JsonSerializer::ValueToString(const SerializedValue& Target, std::ostream& Stream,
	uint32 Depth, WriteOptions Opt)
{
	SerializedData::DataType Type = Target.GetType();
	switch (Type)
	{
	case SerializedData::DataType::Int32:
		Stream << Target.GetInt();
		break;
	case SerializedData::DataType::Byte:
		Stream << int32(Target.GetByte());
		break;
	case SerializedData::DataType::Boolean:
		Stream << (Target.GetBool() ? "true" : "false");
		break;
	case SerializedData::DataType::Float:
		Stream << str::FloatToString(Target.GetFloat());
		break;
	case SerializedData::DataType::String:
		Stream << "\"" << EscapeString(Target.GetString()) << "\"";
		break;
	case SerializedData::DataType::Internal_BinaryTypedArray:
	case SerializedData::DataType::Array:
	{
		Stream << "[";

		if (Opt.Indent)
		{
			Stream << "\n";
		}

		auto& Array = Target.GetArray();
		for (auto i = Array.begin(); i < Array.end(); i++)
		{
			if (Opt.Indent)
			{
				WriteIndent(Depth + 1, Stream);
			}
			ValueToString(*i, Stream, Depth + 1, Opt);
			if (i != Array.end() - 1)
			{
				Stream << ",";
			}
			if (Opt.Indent)
			{
				Stream << "\n";
			}
		}
		if (Opt.Indent)
		{
			WriteIndent(Depth, Stream);
		}
		Stream << "]";
		break;
	}
	case SerializedData::DataType::Object:
		WriteObject(Target.GetObject(), Stream, Depth, Opt);
		break;
	case SerializedData::DataType::Null:
		Stream << "null";
		break;
	case SerializedData::DataType::Vector3:
	case SerializedData::DataType::Vector2:
		throw SerializeException("Cannot serialize vector types with JSON.");
	default:
		break;
	}
}

engine::string engine::JsonSerializer::EscapeString(string In)
{
	string Out;
	Out.reserve(In.size());
	for (auto c : In)
	{
		if (c == '"')
		{
			Out.append("\\\"");
		}
		else if (c == '\n')
		{
			Out.append("\\n");
		}
		else if (c == '\t')
		{
			Out.append("\\t");
		}
		else if (c == '\\')
		{
			Out.append("\\\\");
		}
		else
		{
			Out.push_back(c);
		}
	}

	return Out;
}

engine::string engine::JsonSerializer::ReadWord(std::istream& Stream)
{
	ReadWhitespace(Stream);
	string Out;

	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			break;

		if (New == '\n' || New == '\t' || New == ' ' || New == '\r'
			|| New == ',' || New == ';' || New == ':' || New == '}' || New == ']')
			break;

		Out.push_back(char(New));
	}
	Stream.seekg(-1, Stream.cur);
	return Out;
}

bool engine::JsonSerializer::TryReadChar(std::istream& Stream, char c)
{
	std::streampos StartPos = Stream.tellg();
	while (true)
	{
		int New = Stream.get();

		if (New == EOF)
			break;

		if (New == '\n' || New == '\r' || New == '\t' || New == ' ')
			continue;
		if (New == c)
			return true;
		break;
	}
	Stream.seekg(StartPos);
	return false;
}

void engine::JsonSerializer::ReadWhitespace(std::istream& Stream)
{
	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			break;

		if (New == '\n' || New == '\t' || New == ' ' || New == '\r')
			continue;
		break;
	}
	Stream.seekg(-1, Stream.cur);
}

char engine::JsonSerializer::GetNextChar(std::istream& Stream)
{
	ReadWhitespace(Stream);
	return char(Stream.get());
}

SerializedValue engine::JsonSerializer::FromStream(std::istream& Stream)
{
	return ReadValue(Stream);
}

void engine::JsonSerializer::ReadObject(std::vector<SerializedData>& Object, std::istream& Stream)
{
	if (TryReadChar(Stream, '}'))
	{
		return;
	}

	while (true)
	{
		if (!TryReadChar(Stream, '"'))
		{
			throw SerializeReadException("Expected a string key in JSON object.");
		}
		string Name = ReadString(Stream);

		if (!TryReadChar(Stream, ':'))
		{
			throw SerializeReadException("Expected a colon after key in JSON object.");
		}

		Object.push_back({ Name, ReadValue(Stream) });
		auto Next = GetNextChar(Stream);

		if (Next == ',')
		{
			continue;
		}
		else if (Next == '}')
		{
			break;
		}
		else
		{
			throw SerializeReadException(str::Format("Unexpected '%c' in JSON array.", Next));
		}
	}
}

void engine::JsonSerializer::ReadArray(std::vector<SerializedValue>& Array, std::istream& Stream)
{
	if (TryReadChar(Stream, ']'))
	{
		return;
	}

	while (true)
	{
		Array.push_back(ReadValue(Stream));
		auto Next = GetNextChar(Stream);

		if (Next == ',')
		{
			continue;
		}
		else if (Next == ']')
		{
			break;
		}
		else
		{
			throw SerializeReadException(str::Format("Unexpected '%c' in JSON array.", Next));
		}
	}
}

SerializedValue engine::JsonSerializer::FromFile(string File)
{
	if (!std::filesystem::exists(File) || !std::filesystem::is_regular_file(File))
	{
		throw SerializeReadException(str::Format("Failed reading '%s'. File does not exist or is not a regular file.",
			File.c_str()));
	}

	if (std::filesystem::is_empty(File))
	{
		return {};
	}

	std::ifstream In = std::ifstream(File);
	return ReadValue(In);
}

string engine::JsonSerializer::ReadString(std::istream& Stream)
{
	string Out;
	bool LastWasBackslash = false;
	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			return "";

		if (!LastWasBackslash && New == '"')
		{
			break;
		}
		if (LastWasBackslash && New == 'n')
		{
			Out.push_back('\n');
			LastWasBackslash = false;
			continue;
		}
		if (New == '\\' && !LastWasBackslash)
		{
			LastWasBackslash = true;
			continue;
		}
		else
		{
			LastWasBackslash = false;
		}
		Out.push_back(New);
	}

	return Out;
}

SerializedValue engine::JsonSerializer::ReadValue(std::istream& Stream)
{
	if (TryReadChar(Stream, '{'))
	{
		std::vector<SerializedData> ObjData;
		ReadObject(ObjData, Stream);
		return ObjData;
	}

	if (TryReadChar(Stream, '['))
	{
		std::vector<SerializedValue> ObjData;
		ReadArray(ObjData, Stream);
		return ObjData;
	}

	if (TryReadChar(Stream, '"'))
	{
		return ReadString(Stream);
	}

	string Word = ReadWord(Stream);

	if (Word == "null")
	{
		return SerializedValue();
	}

	if (Word == "false" || Word == "true")
	{
		return SerializedValue(Word == "true");
	}

	try
	{
		return std::stof(Word);
	}
	catch (std::exception& e)
	{
		throw SerializeReadException("Unknown JSON value: " + Word);
	}
}
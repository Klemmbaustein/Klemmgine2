#include "TextSerializer.h"
#include <fstream>
#include <Engine/Log.h>
#include <filesystem>
#include <map>

static const std::map<engine::SerializedData::DataType, engine::string> TypeNames =
{
	{ engine::SerializedData::DataType::Null, "null" },
	{ engine::SerializedData::DataType::Int32, "int" },
	{ engine::SerializedData::DataType::Byte, "byte" },
	{ engine::SerializedData::DataType::Boolean, "bool" },
	{ engine::SerializedData::DataType::Float, "float" },
	{ engine::SerializedData::DataType::Vector3, "vec3" },
	{ engine::SerializedData::DataType::Vector2, "vec2" },
	{ engine::SerializedData::DataType::String, "str" },
	{ engine::SerializedData::DataType::Array, "array" },
	{ engine::SerializedData::DataType::Internal_BinaryTypedArray, "typed_array" },
	{ engine::SerializedData::DataType::Object, "obj" },
};

void engine::TextSerializer::ToStream(const std::vector<SerializedData>& Target, std::ostream& Stream)
{
	WriteObject(Target, Stream, 0);
}

void engine::TextSerializer::ToFile(const std::vector<SerializedData>& Target, string File)
{
	std::ofstream Out = std::ofstream(File);
	ToStream(Target, Out);
	Out.close();
}

void engine::TextSerializer::WriteObject(const std::vector<SerializedData>& Target, std::ostream& Stream, uint32 Depth)
{
	Stream << "{\n";
	for (auto& i : Target)
	{
		WriteIndent(Depth + 1, Stream);
		Stream << "\"" << EscapeString(i.Name) << "\"";
		Stream << ": " << TypeNames.at(i.Value.GetType());
		if (i.Value.GetType() != SerializedData::DataType::Null)
		{
			Stream << " = ";
			ValueToString(i.Value, Stream, Depth + 1);
		}
		Stream << ";\n";
	}
	WriteIndent(Depth, Stream);
	Stream << "}";
}

std::vector<engine::SerializedData> engine::TextSerializer::FromStream(std::istream& Stream)
{
	std::vector<SerializedData> Out;
	ReadObject(Out, Stream);
	return Out;
}

std::vector<engine::SerializedData> engine::TextSerializer::FromFile(string File)
{
	if (!std::filesystem::exists(File))
	{
		return {};
	}

	if (std::filesystem::is_empty(File))
	{
		return {};
	}

	std::ifstream In = std::ifstream(File);
	std::vector<SerializedData> Out;
	ReadObject(Out, In);
	In.close();
	return Out;
}

void engine::TextSerializer::WriteIndent(uint32 Depth, std::ostream& Stream)
{
	for (uint32 i = 0; i < Depth; i++)
	{
		Stream << "\t";
	}
}

void engine::TextSerializer::ValueToString(const SerializedValue& Target, std::ostream& Stream, uint32 Depth)
{
	SerializedData::DataType Type = Target.GetType();
	switch (Type)
	{
	case SerializedData::DataType::Int32:
		Stream << Target.GetInt();
		break;
	case SerializedData::DataType::Byte:
		Stream << Target.GetByte();
		break;
	case SerializedData::DataType::Boolean:
		Stream << Target.GetBool();
		break;
	case SerializedData::DataType::Float:
		Stream << Target.GetFloat();
		break;
	case SerializedData::DataType::Vector3:
		Stream << "<" << Target.GetVector3().ToString() << ">";
		break;
	case SerializedData::DataType::Vector2:
		Stream << "<" << Target.GetVector3().ToString() << ">";
		break;
	case SerializedData::DataType::String:
		Stream << "\"" << EscapeString(Target.GetString()) << "\"";
		break;
	case SerializedData::DataType::Internal_BinaryTypedArray:
	case SerializedData::DataType::Array:
	{
		Stream << "[\n";
		for (const SerializedValue& i : Target.GetArray())
		{
			WriteIndent(Depth + 1, Stream);
			Stream << TypeNames.at(i.GetType()) << ": ";
			ValueToString(i, Stream, Depth + 1);
			Stream << ",\n";
		}
		WriteIndent(Depth, Stream);
		Stream << "]";
		break;
	}
	case SerializedData::DataType::Object:
		WriteObject(Target.GetObject(), Stream, Depth);
		break;
	case SerializedData::DataType::Null:
	default:
		break;
	}
}

engine::string engine::TextSerializer::EscapeString(string In)
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

void engine::TextSerializer::ReadObject(std::vector<SerializedData>& Object, std::istream& Stream)
{
	if (!TryReadChar(Stream, '{'))
	{
		throw SerializeReadException(str::Format("object does not start with '{' (starts with: '%c')", GetNextChar(Stream)));
	}

	while (true)
	{
		if (TryReadChar(Stream, '}'))
		{
			break;
		}

		string Name = ReadString(Stream);

		if (!TryReadChar(Stream, ':'))
		{
			char c = GetNextChar(Stream);
			throw SerializeReadException(str::Format("Expected a ':' after \"%s\", got '%c' (hex 0x%x)", Name.c_str(), c, c));
		}
		std::string Type = ReadWord(Stream);
		SerializedValue Value;
		if (Type != "null")
		{
			if (!TryReadChar(Stream, '='))
			{
				throw SerializeReadException(str::Format("Expected a '=' after the type, got '%c'", GetNextChar(Stream)));
			}
			Value = ReadValue(Stream, GetTypeFromString(Type));
		}
		if (!TryReadChar(Stream, ';'))
		{
			throw SerializeReadException(str::Format("Expected a ';' after value, got '%c'", GetNextChar(Stream)));
		}
		Object.push_back(SerializedData(Name, Value));
	}
}

engine::SerializedValue engine::TextSerializer::ReadValue(std::istream& Stream, SerializedData::DataType Type)
{
	switch (Type)
	{
	case engine::SerializedData::DataType::Int32:
		return std::stoi(ReadWord(Stream));

	case engine::SerializedData::DataType::Byte:
		return uByte(std::stoi(ReadWord(Stream)));

	case engine::SerializedData::DataType::Boolean:
		return bool(std::stoi(ReadWord(Stream)));

	case engine::SerializedData::DataType::Float:
		return std::stof(ReadWord(Stream));

	case engine::SerializedData::DataType::Vector3:
		return Vector3::FromString(ReadVector(Stream));
	case engine::SerializedData::DataType::Vector2:
		return SerializedValue(Vector2::FromString(ReadVector(Stream)));
	case engine::SerializedData::DataType::String:
		return ReadString(Stream);

	case SerializedData::DataType::Internal_BinaryTypedArray:
	case engine::SerializedData::DataType::Array:
	{
		std::vector<SerializedValue> Array;
		if (!TryReadChar(Stream, '['))
		{
			throw SerializeReadException(str::Format("Invalid array. Should start with [, starts with %c", GetNextChar(Stream)));
		}
		while (!TryReadChar(Stream, ']'))
		{
			ReadWhitespace(Stream);
			char TypeBuffer[128];
			Stream.get(TypeBuffer, sizeof(TypeBuffer) - 1, ':');
			TypeBuffer[sizeof(TypeBuffer) - 1] = 0;

			if (!TryReadChar(Stream, ':'))
			{
				throw SerializeReadException(str::Format("Expected a ':' after '%s', got '%c'", TypeBuffer, GetNextChar(Stream)));
			}
			Array.push_back(ReadValue(Stream, GetTypeFromString(TypeBuffer)));
			TryReadChar(Stream, ',');
		}
		return Array;
	}
	case engine::SerializedData::DataType::Object:
	{
		std::vector<SerializedData> Object;
		ReadObject(Object, Stream);
		return Object;
	}
	case engine::SerializedData::DataType::Null:
	default:
		break;
	}
	return SerializedValue();
}

engine::string engine::TextSerializer::ReadString(std::istream& Stream)
{
	while (true)
	{
		int fail = Stream.fail();
		int New = Stream.get();
		if (New == EOF)
			return "";

		if (New == '\n' || New == '\t' || New == ' ' || New == '\r')
			continue;
		if (New == '"')
			break;
		throw SerializeReadException(str::Format("Unexpected character '%c' (hex %x)", char(New), New));
	}

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

engine::string engine::TextSerializer::ReadVector(std::istream& Stream)
{
	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			return "";

		if (New == '\n' || New == '\t' || New == ' ' || New == '\r')
			continue;
		if (New == '<')
			break;
		throw SerializeReadException(str::Format("Unexpected character '%c' (hex %x)", char(New), New));
	}

	string Out;
	bool LastWasBackslash = false;
	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			return "";

		if (!LastWasBackslash && New == '>')
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

engine::string engine::TextSerializer::ReadWord(std::istream& Stream)
{
	ReadWhitespace(Stream);
	string Out;

	while (true)
	{
		int New = Stream.get();
		if (New == EOF)
			break;

		if (New == '\n' || New == '\t' || New == ' ' || New == '\r' || New == ',' || New == ';' || New == ':')
			break;

		Out.push_back(char(New));
	}
	Stream.seekg(-1, Stream.cur);
	return Out;
}

bool engine::TextSerializer::TryReadChar(std::istream& Stream, char c)
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

void engine::TextSerializer::ReadWhitespace(std::istream& Stream)
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

char engine::TextSerializer::GetNextChar(std::istream& Stream)
{
	ReadWhitespace(Stream);
	return char(Stream.get());
}

engine::SerializedData::DataType engine::TextSerializer::GetTypeFromString(string Str)
{
	for (auto& i : TypeNames)
	{
		if (i.second == Str)
			return i.first;
	}
	Log::Warn(str::Format("Unkown type: %s", Str.c_str()));
	return SerializedData::DataType::Null;
}

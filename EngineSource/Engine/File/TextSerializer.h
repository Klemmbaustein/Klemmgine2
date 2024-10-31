#pragma once
#include <vector>
#include "SerializedData.h"
#include <ostream>
#include <istream>

namespace engine
{
	class TextSerializer
	{
	public:
		static void ToStream(const std::vector<SerializedData>& Target, std::ostream& Stream);
		static void ToFile(const std::vector<SerializedData>& Target, string File);
		static void WriteObject(const std::vector<SerializedData>& Target, std::ostream& Stream, uint32 Depth);

		static std::vector<SerializedData> FromStream(std::istream& Stream);
		static std::vector<SerializedData> FromFile(string File);
	private:
		static void WriteIndent(uint32 Depth, std::ostream& Stream);
		static void ValueToString(const SerializedValue& Target, std::ostream& Stream, uint32 Depth);

		static string EscapeString(string In);

		static void ReadObject(std::vector<SerializedData>& Object, std::istream& Stream);
		static SerializedValue ReadValue(std::istream& Stream, SerializedData::DataType Type);
		static string ReadString(std::istream& Stream);
		static string ReadVector(std::istream& Stream);
		static string ReadWord(std::istream& Stream);
		static bool TryReadChar(std::istream& Stream, char c);
		static void ReadWhitespace(std::istream& Stream);

		static SerializedData::DataType GetTypeFromString(string Str);
	};
}
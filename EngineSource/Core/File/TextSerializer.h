#pragma once
#include <vector>
#include "SerializedData.h"
#include <ostream>
#include <istream>

namespace engine
{
	/**
	* @brief
	* A serializer that converts engine::SerializedData into a string representation.
	*
	* The representation looks kind of like JSON with explicit type annotations.
	* Like JSON, it currently doesn't support comments.
	*
	* All functions of this class might throw a SerializeReadException.
	*
	* @ingroup serialize
	*/
	class TextSerializer
	{
	public:
		/**
		* @brief
		* Writes the contents of the SerializedData objects to the given C++ stream.
		*
		* This can be used with a std::stringstream to write it to a string, or std::iostream to write it
		* to the standard output.
		*
		* This function directly calls WriteObject() with Depth = 0.
		*/
		static void ToStream(const std::vector<SerializedData>& Target, std::ostream& Stream);
		/**
		* @brief
		* Writes the contents of the SerializedData objects to a file with the given file path.
		*/
		static void ToFile(const std::vector<SerializedData>& Target, string File);
		/**
		* @brief
		* Writes the contents of the SerializedData objects to the given C++ stream.
		*
		* Unlike ToStream, this function accepts a 'Depth' argument, that controls
		* the indentation of the written data.
		*/
		static void WriteObject(const std::vector<SerializedData>& Target, std::ostream& Stream, uint32 Depth);

		/**
		* @brief
		* Reads a serialized object from the given stream.
		*
		* Will throw a SerializeReadException when it encounters an error with reading the data.
		*/
		static std::vector<SerializedData> FromStream(std::istream& Stream);
		/**
		* @brief
		* Reads a serialized object from a file with the given path.
		*
		* Will throw a SerializeReadException when it encounters an error with reading the data.
		*/
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
		static char GetNextChar(std::istream& Stream);

		static SerializedData::DataType GetTypeFromString(string Str);
	};
}
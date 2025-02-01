#pragma once
#include <vector>
#include "SerializedData.h"
#include <cstring>

namespace engine
{

	/**
	* @brief
	* Class to convert a SerializedValue objects to binary files and .
	* 
	* @ingroup serialize
	*/
	class BinarySerializer
	{
	public:
		/// The current version of the binary format used by the engine.
		static const string FORMAT_VERSION;

		/**
		* @brief
		* Converts the given serialized data structure to a binary array.
		* 
		* @param Target
		* The serialized object to be written.
		* 
		* @param Out
		* The Output bytes.
		* 
		* @param FormatIdentifier
		* The name of the format written to the binary data.
		* 
		* @see BinarySerializer::ToFile()
		*/
		static void ToBinaryData(const std::vector<SerializedData>& Target, std::vector<uByte>& Out, string FormatIdentifier = "k2b");

		/**
		* @brief
		* Converts the given serialized value to bytes.
		*/
		static void ValueToBinaryData(const SerializedValue& Target, std::vector<uByte>& Out, SerializedData::DataType Type = SerializedData::DataType::Null);

		/**
		* @brief
		* Converts the given serialized data structure to a binary format and writes them to a file.
		* 
		* @param Target
		* The serialized object to be written.
		*
		* @param FileName
		* The output file.
		*
		* @param FormatIdentifier
		* The name of the format written to the binary data.
		* 
		* @see BinarySerializer::ToBinaryData()
		*/
		static void ToFile(const std::vector<SerializedData>& Target, string FileName, string FormatIdentifier = "k2b");

		/**
		* @brief
		* Loads serialized data from the given file.
		* 
		* @param File
		* The file from which the binary data should be read.
		* 
		* @param FormatIdentifier
		* The format that the file should use. If the format doesn't match, this function will fail.
		* 
		* @return
		* A vector of serialized data read from the file.
		* 
		* @throw SerializeReadException
		*/
		static std::vector<SerializedData> FromFile(string File, string FormatIdentifier);

		/**
		* @brief
		* Loads serialized data from the given file.
		* 
		* Identical to std::vector<SerializedData> FromFile(string File, string FormatIdentifier),
		* but instead of copying the result, the destination vector is passed as a reference.
		*
		* @param File
		* The file from which the binary data should be read.
		*
		* @param FormatIdentifier
		* The format that the file should use. If the format doesn't match, this function will fail.
		*
		* @return
		* A vector of serialized data read from the file.
		*
		* @throw SerializeReadException
		*/
		static void FromFile(string File, string FormatIdentifier, std::vector<SerializedData>& To);
	private:
		struct BinaryStream
		{
			std::vector<uByte> Bytes;
			size_t StreamPos = 0;
			uByte GetByte();
			template<typename T> T Get()
			{
				if (Bytes.size() < StreamPos + sizeof(T))
				{
					return T();
				}
				T Out = T();
				memcpy(&Out, &Bytes[StreamPos], sizeof(T));
				StreamPos += sizeof(T);
				return Out;
			}
			bool Empty() const;
		};

		static void ReadSerializedData(BinaryStream& From, SerializedData& Out);
		static void ReadValue(BinaryStream& From, SerializedValue& To, SerializedData::DataType Type = SerializedData::DataType::Null);

		static void WriteString(string Str, std::vector<uByte>& Out);
		static void CopyTo(void* Data, size_t Size, std::vector<uByte>& Out);

		static void ReadString(BinaryStream& From, string& Out);
	};
}
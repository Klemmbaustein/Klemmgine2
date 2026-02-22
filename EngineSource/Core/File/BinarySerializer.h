#pragma once
#include <vector>
#include "SerializedData.h"
#include "BinaryStream.h"
#include <cstring>

namespace engine
{
	/**
	* @brief
	* Class to convert SerializedData objects to binary files and the other way around.
	*
	* All functions of this class might throw a SerializeReadException.
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
		static void ToBinaryData(const std::vector<SerializedData>& Target, IBinaryStream* Out, string FormatIdentifier = "k2b");

		/**
		* @brief
		* Converts the given serialized value to bytes.
		*/
		static void ValueToBinaryData(const SerializedValue& Target, IBinaryStream* Out,
			SerializedData::DataType Type = SerializedData::DataType::Null, bool AllowTypeChange = true);

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
		static std::vector<SerializedData> FromFile(string File, string FormatIdentifier = "k2b");

		static std::vector<SerializedData> FromStream(IBinaryStream* Stream, string FormatIdentifier = "k2b");

		static void FromStream(IBinaryStream* Stream, std::vector<SerializedData>& To, string FormatIdentifier = "k2b");

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

		static void ReadSerializedData(IBinaryStream* From, SerializedData& Out);
		static void ReadValue(IBinaryStream* From, SerializedValue& To, SerializedData::DataType Type = SerializedData::DataType::Null);
	};
}
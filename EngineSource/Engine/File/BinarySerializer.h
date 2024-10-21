#pragma once
#include <vector>
#include "SerializedData.h"

namespace engine
{
	class BinarySerializer
	{
	public:
		static const string FormatVersion;

		static void ToBinaryData(const std::vector<SerializedData>& Target, std::vector<uByte>& Out, string FormatIdentifier = "k2b");
		static void ValueToBinaryData(const SerializedValue& Target, std::vector<uByte>& Out);

		static void ToFile(const std::vector<SerializedData>& Target, string FileName, string FormatIdentifier = "k2b");

		static SerializedData FromFile(string File, string FormatIdentifier);
	private:
		struct BinaryStream
		{
			std::vector<uByte> Bytes;
			size_t StreamPos = 0;
			uByte GetByte();
			bool Empty();
		};

		static void WriteString(string Str, std::vector<uByte>& Out);
		static void CopyTo(void* Data, size_t Size, std::vector<uByte>& Out);

		static string ReadString(BinaryStream& From);
	};
}
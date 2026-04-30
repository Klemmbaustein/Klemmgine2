#pragma once
#include <Core/File/BinaryStream.h>

namespace engine
{
	class BitStreamReader
	{
	public:

		BitStreamReader(IBinaryStream* Stream)
		{
			this->Stream = Stream;
		}

		template<size_t n>
		static uint64 SwapEndian(uint32 Value)
		{
			union
			{
				uint32 Integer;
				uByte Bytes[sizeof(uint64)]{ 0 };
			} source, dest;

			source.Integer = Value;

			for (size_t i = 0; i < sizeof(uint64); i++)
			{
				if (i >= n)
				{
					dest.Bytes[i] = 0;
				}
				else
				{
					dest.Bytes[i] = source.Bytes[n - i - 1];
				}
			}
			return dest.Integer;
		}

		bool ReadBit()
		{
			if (CurrentBytePosition > 7)
			{
				CurrentByte = Stream->GetByte();
				CurrentBytePosition = 0;
				ByteCount++;
			}

			bool Value = CurrentByte & 0b10000000;
			CurrentByte <<= 1;
			CurrentBytePosition++;

			return Value;
		}

		bool ReadBytes(uByte* To, size_t Count)
		{
			return Stream->Read(To, Count);
		}

		template<size_t n>
		uint64 ReadBits()
		{
			uint64 Result = 0;

			for (size_t i = 0; i < n; i++)
			{
				bool v = ReadBit();
				Result |= uint64(v) << (n - i - 1);
			}

			return Result;
		}

		uint64 ReadNBits(size_t n)
		{
			uint64 Result = 0;

			for (size_t i = 0; i < n; i++)
			{
				bool v = ReadBit();
				Result |= uint64(v) << (n - i - 1);
			}

			return Result;
		}

		template<typename T>
		T Get()
		{
			AlignToByte();
			return Stream->Get<T>();
		}

		void AlignToByte()
		{
			CurrentBytePosition = 8;
		}

	private:
		IBinaryStream* Stream;

		size_t ByteCount = 0;
		uByte CurrentByte = 0;
		uByte CurrentBytePosition = 8;
	};
}
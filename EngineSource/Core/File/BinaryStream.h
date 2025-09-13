#pragma once
#include <Core/Types.h>
#include <cstdio>

namespace engine
{
	class IBinaryStream
	{
	public:
		virtual ~IBinaryStream() = default;

		virtual bool IsReadOnly() const = 0;
		virtual bool IsWriteOnly() const = 0;
		virtual bool IsEmpty() const = 0;
		virtual size_t GetSize() const = 0;

		virtual bool Read(uByte* To, size_t Size) = 0;
		virtual void Write(uByte* Buffer, size_t Size) = 0;

		uByte GetByte()
		{
			uByte Out;
			if (!Read(&Out, sizeof(Out)))
			{
				return 0;
			}
			return Out;
		}

		template<typename T>
		T Get()
		{
			T Out = T();
			if (!Read((uByte*)&Out, sizeof(Out)))
			{
				return T();
			}
			return Out;
		}

		void WriteByte(uByte Byte)
		{
			Write(&Byte, sizeof(Byte));
		}

		template<typename T>
		void WriteValue(const T& Value)
		{
			Write((uByte*)&Value, sizeof(Value));
		}

		void WriteString(string Str)
		{
			Write((uByte*)Str.data(), Str.size());
			WriteByte(0);
		}

		void ReadString(string& Out)
		{
			uByte Char;
			while (!IsEmpty() && (Char = GetByte()))
			{
				Out.push_back(char(Char));
			}
		}

		[[nodiscard]]
		string ReadString()
		{
			string Result;
			ReadString(Result);
			return Result;
		}
	};

	class FileStream : public IBinaryStream
	{
	public:

		FileStream(string FilePath, bool Read);
		~FileStream() override;

		virtual bool IsReadOnly() const override;
		virtual bool IsWriteOnly() const override;
		virtual bool IsEmpty() const override;
		virtual size_t GetSize() const override;

		virtual bool Read(uByte* To, size_t Size) override;
		virtual void Write(uByte* Buffer, size_t Size) override;

	private:
		FILE* FromFile = nullptr;
		bool ReadFile = false;
		size_t ReadSize = 0;
	};

	class BufferStream : public IBinaryStream
	{
	public:
		BufferStream();
		~BufferStream() override;

		virtual bool IsReadOnly() const override;
		virtual bool IsWriteOnly() const override;
		virtual bool IsEmpty() const override;
		virtual size_t GetSize() const override;

		virtual bool Read(uByte* To, size_t Size) override;
		virtual void Write(uByte* Buffer, size_t Size) override;

		const std::vector<uByte>& GetBuffer() const;

		void ResetStreamPosition();

	private:
		size_t StreamPosition = 0;
		std::vector<uByte> Buffer;
	};

	class ReadOnlyBufferStream : public IBinaryStream
	{
	public:
		ReadOnlyBufferStream();
		ReadOnlyBufferStream(const uByte* Data, size_t Size, bool FreeOnClose);
		ReadOnlyBufferStream(const ReadOnlyBufferStream&) = delete;
		~ReadOnlyBufferStream() override;

		virtual bool IsReadOnly() const override;
		virtual bool IsWriteOnly() const override;
		virtual bool IsEmpty() const override;
		virtual size_t GetSize() const override;

		virtual bool Read(uByte* To, size_t Size) override;
		virtual void Write(uByte* Buffer, size_t Size) override;

		const uByte* GetData();

	private:
		bool FreeOnClose = false;
		const uByte* Data = nullptr;
		size_t Size = 0, StreamPos = 0;
	};

}
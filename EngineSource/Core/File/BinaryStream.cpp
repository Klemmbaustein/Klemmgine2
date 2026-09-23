#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "BinaryStream.h"
#include "Core/Log.h"
#include <cstring>

using namespace engine;

FileStream::FileStream(string FilePath, bool Read)
{
	if (Read)
	{
		FromFile = std::fstream(FilePath, std::ios::in | std::ios::binary);
	}
	else
	{
		FromFile = std::fstream(FilePath, std::ios::out | std::ios::binary);
	}

	ReadFile = Read;

	if (ReadFile)
	{
		FromFile.seekg(0, std::ios::end);
		ReadSize = size_t(FromFile.tellg());
		FromFile.seekg(0, std::ios::beg);
	}
}

FileStream::~FileStream()
{
	FromFile.close();
}

bool FileStream::IsReadOnly() const
{
	return ReadFile;
}

bool FileStream::IsWriteOnly() const
{
	return !ReadFile;
}

bool FileStream::IsEmpty() const
{
	return FromFile.eof() || ReadSize == 0;
}

bool FileStream::Read(uByte* To, size_t Size)
{
	if (!To)
	{
		FromFile.seekg(Size, std::ios::cur);

		return FromFile.eof();
	}

	FromFile.read(reinterpret_cast<char*>(To), Size);
	if (FromFile.eof() || FromFile.bad() || FromFile.fail())
	{
		if (!FromFile.eof() && (FromFile.bad() || FromFile.fail()))
		{
			Log::Error("Read failed");
		}
		return false;
	}

	return true;
}

void FileStream::Write(uByte* Buffer, size_t Size)
{
	FromFile.write(reinterpret_cast<char*>(Buffer), Size);
}

size_t FileStream::GetSize() const
{
	return ReadSize == SIZE_MAX ? 0 : ReadSize;
}

BufferStream::BufferStream()
{
}

BufferStream::BufferStream(const uByte* Data, size_t Size)
{
	this->Buffer = std::vector<uByte>();
	this->Buffer.resize(Size);
	memcpy(this->Buffer.data(), Data, Size);
}

BufferStream::~BufferStream()
{
}

bool BufferStream::IsReadOnly() const
{
	return false;
}

bool BufferStream::IsWriteOnly() const
{
	return false;
}

bool BufferStream::IsEmpty() const
{
	return this->Buffer.size() <= StreamPosition;
}

bool BufferStream::Read(uByte* To, size_t Size)
{
	if (this->Buffer.size() < Size + StreamPosition)
		return false;

	if (To)
	{
		memcpy(To, &this->Buffer[StreamPosition], Size);
	}
	StreamPosition += Size;
	return true;
}

void BufferStream::Write(uByte* Buffer, size_t Size)
{
	if (Size == 0)
		return;

	StreamPosition = this->Buffer.size();
	this->Buffer.resize(StreamPosition + Size);
	memcpy(&this->Buffer[StreamPosition], Buffer, Size);
}

const std::vector<uByte>& BufferStream::GetBuffer() const
{
	return Buffer;
}

void BufferStream::ResetStreamPosition()
{
	StreamPosition = 0;
}

size_t BufferStream::GetSize() const
{
	return Buffer.size();
}

engine::ReadOnlyBufferStream::ReadOnlyBufferStream()
{
}

engine::ReadOnlyBufferStream::ReadOnlyBufferStream(const uByte* Data, size_t Size, bool FreeOnClose)
{
	this->Data = Data;
	this->Size = Size;
	this->FreeOnClose = FreeOnClose;
}
engine::ReadOnlyBufferStream::~ReadOnlyBufferStream()
{
	OnClose.Invoke();
	if (FreeOnClose)
		delete[] Data;
}

bool engine::ReadOnlyBufferStream::IsReadOnly() const
{
	return true;
}

bool engine::ReadOnlyBufferStream::IsWriteOnly() const
{
	return false;
}

bool engine::ReadOnlyBufferStream::IsEmpty() const
{
	return Size <= StreamPos;
}

size_t engine::ReadOnlyBufferStream::GetSize() const
{
	return Size;
}

bool engine::ReadOnlyBufferStream::Read(uByte* To, size_t Size)
{
	if (this->Size < Size + StreamPos)
		return false;

	if (To)
	{
		memcpy(To, &this->Data[StreamPos], Size);
	}
	StreamPos += Size;
	return true;
}

void engine::ReadOnlyBufferStream::Write(uByte* Buffer, size_t Size)
{
}

const uByte* ReadOnlyBufferStream::GetData()
{
	return Data;
}

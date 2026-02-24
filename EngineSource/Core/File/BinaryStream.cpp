#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "BinaryStream.h"
#include "Core/Log.h"
#include <cstring>

using namespace engine;

FileStream::FileStream(string FilePath, bool Read)
{
	FromFile = fopen(FilePath.c_str(), Read ? "rb" : "wb");
	if (!FromFile)
	{
		Log::Warn(FilePath + ": " + strerror(errno));
		return;
	}

	ReadFile = Read;

	if (ReadFile)
	{
		fseek(FromFile, 0, SEEK_END);
		ReadSize = size_t(ftell(FromFile));
		rewind(FromFile);
	}
}

FileStream::~FileStream()
{
	fclose(FromFile);
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
	return feof(FromFile);
}

bool FileStream::Read(uByte* To, size_t Size)
{
	size_t read = fread(To, Size, 1, FromFile);
	if (read != 1)
	{
		if (errno != 0)
		{
			Log::Error(str::Format("Read failed: %s, is empty: %i", strerror(errno), IsEmpty()));
		}
		return false;
	}

	return true;
}

void FileStream::Write(uByte* Buffer, size_t Size)
{
	fwrite(Buffer, Size, 1, FromFile);
}

size_t FileStream::GetSize() const
{
	return ReadSize;
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
	return Buffer.empty();
}

bool BufferStream::Read(uByte* To, size_t Size)
{
	if (this->Buffer.size() < Size + StreamPosition)
		return false;

	memcpy(To, &this->Buffer[StreamPosition], Size);
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

	memcpy(To, &this->Data[StreamPos], Size);
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

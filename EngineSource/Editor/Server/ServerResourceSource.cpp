#include "ServerResourceSource.h"
#include <Core/File/FileUtil.h>

using namespace engine::editor;
using namespace engine;

engine::editor::ServerResourceSource::ServerResourceSource(ServerConnection* Connection)
{
	this->Connection = Connection;
}

bool engine::editor::ServerResourceSource::FileExists(string Path)
{
	for (auto& i : this->Connection->Files)
	{
		if (i == Path)
		{
			return true;
		}
	}

	return false;
}

IBinaryStream* engine::editor::ServerResourceSource::GetFile(string Path)
{
	return new ServerResourceStream(Path, Connection);
}

std::map<string, string> engine::editor::ServerResourceSource::GetFiles()
{
	std::map<string, string> Files;
	for (auto& i : this->Connection->Files)
	{
		Files.insert({ file::FileName(i), i });
	}

	return Files;
}

engine::editor::ServerResourceStream::ServerResourceStream(string Path, ServerConnection* Connection)
	: Connection(Connection)
{
	ListenedFile = Path;
	Connection->GetFile(Path, [this](ReadOnlyBufferStream* Buffer, bool IsLast) {
		{
			std::unique_lock lk{ QueueMutex };
			if (!Buffer->IsEmpty())
			{
				CurrentChunks.push(Buffer);
			}
			ReachedLast = IsLast;
		}

		cv.notify_one();
	});
}

engine::editor::ServerResourceStream::~ServerResourceStream()
{
	if (CurrentChunk)
	{
		delete CurrentChunk;
	}
}

bool engine::editor::ServerResourceStream::IsReadOnly() const
{
	return true;
}

bool engine::editor::ServerResourceStream::IsWriteOnly() const
{
	return false;
}

bool engine::editor::ServerResourceStream::IsEmpty() const
{
	return ReachedLast && CurrentChunks.empty() && CurrentChunk->IsEmpty();
}

size_t engine::editor::ServerResourceStream::GetSize() const
{
	return size_t();
}

bool engine::editor::ServerResourceStream::Read(uByte* To, size_t Size)
{
	size_t Read = 0;

	while (Read < Size)
	{
		if (CurrentChunk)
		{
			size_t Remaining = std::min(CurrentChunk->GetSize() - CurrentChunk->StreamPosition, Size - Read);
			if (Remaining)
			{
				CurrentChunk->Read(To + Read, Remaining);
				//std::cout << string((char*)To, Size);
			}
			Read += Remaining;
		}

		if (!CurrentChunk || CurrentChunk->IsEmpty())
		{
			NextChunk();
		}
		if (!CurrentChunk)
		{
			return Read == Size;
		}
	}

	return true;
}

void engine::editor::ServerResourceStream::Write(uByte* Buffer, size_t Size)
{
}

void engine::editor::ServerResourceStream::NextChunk()
{
	if (CurrentChunk)
	{
		delete CurrentChunk;
		CurrentChunk = nullptr;
	}

	if (ReachedLast)
	{
		if (!CurrentChunks.empty())
		{
			CurrentChunk = CurrentChunks.front();
			CurrentChunks.pop();
			return;
		}
	}

	{
		std::unique_lock lk{ QueueMutex };
		if (!CurrentChunks.empty())
		{
			CurrentChunk = CurrentChunks.front();
			CurrentChunks.pop();
			return;
		}
	}
	while (!ReachedLast && !CurrentChunk)
	{
		if (!ReachedLast)
		{
			std::unique_lock lk{ WaitMutex };
			cv.wait(lk);
		}
		if (!CurrentChunks.empty())
		{
			CurrentChunk = CurrentChunks.front();
			CurrentChunks.pop();
		}
	}
}

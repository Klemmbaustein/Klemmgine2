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

ReadOnlyBufferStream* engine::editor::ServerResourceSource::GetFile(string Path)
{
	std::condition_variable cv;
	std::mutex m;
	bool GotResult = false;
	ReadOnlyBufferStream* Result = nullptr;

	this->Connection->GetFile(Path, [&](ReadOnlyBufferStream* Buffer) {
		GotResult = true;
		Result = Buffer;

		cv.notify_one();
	});

	std::unique_lock lk(m);
	cv.wait(lk, [&] { return GotResult; });

	return Result;
}

std::map<string, string> engine::editor::ServerResourceSource::GetFiles()
{
	std::map<string, string> Files;
	for (auto& i : this->Connection->Files)
	{
		Files.insert({file::FileName(i), i});
	}

	return Files;
}

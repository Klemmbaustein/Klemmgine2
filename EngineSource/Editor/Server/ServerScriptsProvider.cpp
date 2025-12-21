#include "ServerScriptsProvider.h"
#include <condition_variable>

using namespace engine;

engine::editor::ServerScriptProvider::ServerScriptProvider(ServerConnection* Connection)
{
	this->Connection = Connection;
}

std::vector<string> engine::editor::ServerScriptProvider::GetFiles()
{
	return { "Scripts/test.ds" };
}

string engine::editor::ServerScriptProvider::GetFile(string Name)
{
	std::condition_variable cv;
	std::mutex m;
	bool GotResult = false;
	ReadOnlyBufferStream* Result = nullptr;

	this->Connection->GetFile(Name, [&](ReadOnlyBufferStream* Buffer) {
		GotResult = true;
		Result = Buffer;

		cv.notify_one();
	});

	std::unique_lock lk(m);
	cv.wait(lk, [&GotResult] { return GotResult; });

	return Result->ReadString();
}

#include <Engine/File/Resource.h>
#include "ServerConnection.h"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace engine::editor
{
	class ServerResourceStream : public IBinaryStream
	{
	public:

		ServerResourceStream(string Path, ServerConnection* Connection);
		~ServerResourceStream();

		// Inherited via IBinaryStream
		bool IsReadOnly() const override;
		bool IsWriteOnly() const override;
		bool IsEmpty() const override;
		size_t GetSize() const override;
		bool Read(uByte* To, size_t Size) override;
		void Write(uByte* Buffer, size_t Size) override;

		std::queue<ReadOnlyBufferStream*> CurrentChunks;
		ReadOnlyBufferStream* CurrentChunk = nullptr;

		void NextChunk();

		std::condition_variable cv;
		std::mutex WaitMutex;
		std::mutex QueueMutex;
		ServerConnection* Connection;
		bool ReachedLast = false;
		std::string ListenedFile;
	};

	class ServerResourceSource : public resource::ResourceSource
	{
	public:

		ServerResourceSource(ServerConnection* Connection);
		bool FileExists(string Path) override;
		IBinaryStream* GetFile(string Path) override;

		std::map<string, string> GetFiles() override;

		ServerConnection* Connection = nullptr;
	};
}
#pragma once
#include <Core/Types.h>
#include <Core/Event.h>

namespace engine
{
	class FileWatcherPlatformData;

	enum class FileChangeType
	{
		Removed,
		Created,
		Modified,
	};

	struct FileChange
	{
		FileChangeType Type = FileChangeType::Modified;
		string FilePath = "";
	};

	class FileWatcher
	{
	public:

		FileWatcher(string Directory);
		~FileWatcher();

		void Update();

		Event<FileChange> OnFileChanged;

	private:
#if WINDOWS
		void WaitForNextChanges();
#else
		void ListenToDirectory(string Name);
		void StopListenToDirectory(string Name);
#endif
		void FlushEvents();
		size_t LastChangeTime = 0;
		size_t Time = 0;
		std::map<string, std::vector<FileChange>> Changes;

		FileWatcherPlatformData* Platform = nullptr;
	};
}
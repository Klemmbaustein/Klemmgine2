#include "FileWatcher.h"
#include <Core/Platform/Platform.h>
#include <Core/Log.h>
#if WINDOWS
#include <Windows.h>

class engine::FileWatcherPlatformData
{
public:
	HANDLE DirectoryHandle = nullptr;
	HANDLE OverlappedEvent = nullptr;
	OVERLAPPED OverlappedWriteData{};
	uByte ChangeBuffer[4042]{};
};
#else
#include <sys/inotify.h>
#include <sys/poll.h>
#include <unistd.h>
#include <map>
#include <filesystem>

class engine::FileWatcherPlatformData
{
public:
	struct
	{
		int INotifyFileDescriptor = 0;
		std::map<int, string> DescriptorMap;
	} Main;
	uByte ChangeBuffer[4096 - sizeof(Main)]{};
};

#endif

engine::FileWatcher::FileWatcher(string Directory)
{
#if WINDOWS
	this->Platform = new FileWatcherPlatformData();

	Platform->DirectoryHandle = CreateFileW(platform::StrToWstr(Directory).c_str(),
		FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

	if (Platform->DirectoryHandle == INVALID_HANDLE_VALUE)
	{
		Log::Error(platform::GetLastErrorString());
	}	DWORD ReturnedBytes = 0;

	Platform->OverlappedEvent = CreateEventW(nullptr, false, false, nullptr);
	Platform->OverlappedWriteData.hEvent = Platform->OverlappedEvent;

	WaitForNextChanges();
#else
	this->Platform = new FileWatcherPlatformData();

	Platform->Main.INotifyFileDescriptor = inotify_init();

	ListenToDirectory(Directory);

	for (auto& item : std::filesystem::recursive_directory_iterator(Directory))
	{
		if (item.is_directory())
		{
			ListenToDirectory(item.path().string());
		}
	}
#endif
}

engine::FileWatcher::~FileWatcher()
{
#if WINDOWS
	CloseHandle(Platform->DirectoryHandle);
	CloseHandle(Platform->OverlappedEvent);
#else
	close(Platform->Main.INotifyFileDescriptor);
#endif
}

void engine::FileWatcher::Update()
{
#if WINDOWS
	auto Result = WaitForSingleObject(Platform->OverlappedEvent, 0);

	if (Result != WAIT_OBJECT_0)
	{
		Time++;
		// After 10 frames of no changes, flush the changes and notify listeners
		// (Changes are triggered for every single write to a file, so notifying directly
		// after each result would give multiple duplicate events for a single file save operation)
		if (LastChangeTime + 10 < Time)
		{
			FlushEvents();
		}

		return;
	}

	DWORD ReturnedBytes = 0;
	GetOverlappedResult(Platform->DirectoryHandle, &Platform->OverlappedWriteData, &ReturnedBytes, false);

	uByte* CurrentPosition = Platform->ChangeBuffer;

	while (true)
	{
		FILE_NOTIFY_INFORMATION* Info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(CurrentPosition);

		FileChange NewChange;

		switch (Info->Action)
		{
		case FILE_ACTION_RENAMED_NEW_NAME:
		case FILE_ACTION_ADDED:
			NewChange.Type = FileChangeType::Created;
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
		case FILE_ACTION_REMOVED:
			NewChange.Type = FileChangeType::Removed;
			break;
		case FILE_ACTION_MODIFIED:
			NewChange.Type = FileChangeType::Modified;
			break;
		}

		NewChange.FilePath = platform::WstrToStr(std::wstring(Info->FileName, Info->FileNameLength / sizeof(wchar_t)));

		Changes[NewChange.FilePath].emplace_back(std::move(NewChange));

		if (Info->NextEntryOffset)
		{
			CurrentPosition += Info->NextEntryOffset;
		}
		else
		{
			break;
		}
	}
	LastChangeTime = Time;

	WaitForNextChanges();
#else

	pollfd fd{};
	fd.fd = Platform->Main.INotifyFileDescriptor;
	fd.events = POLLIN;
	fd.revents = 0;

	int Result = poll(&fd, 1, 0);

	if (Result == -1)
	{
		Log::Error(platform::GetLastErrorString());
	}
	else if (Result > 0)
	{
		ssize_t Count = read(Platform->Main.INotifyFileDescriptor, Platform->ChangeBuffer, sizeof(Platform->ChangeBuffer));

		uByte* End = Platform->ChangeBuffer + Count;
		uByte* Position = Platform->ChangeBuffer;

		while (Position < End)
		{
			inotify_event* Event = reinterpret_cast<inotify_event*>(Position);
			FileChange NewChange;

			NewChange.FilePath = Event->len ? Platform->Main.DescriptorMap.at(Event->wd) + "/" + string(Event->name, Event->len - 1)
				: Platform->Main.DescriptorMap.at(Event->wd);

			Position += sizeof(inotify_event) + Event->len;

			if (std::filesystem::is_directory(NewChange.FilePath))
			{
				if (Event->mask & IN_CREATE)
				{
					ListenToDirectory(NewChange.FilePath);
				}
				if (Event->mask & IN_DELETE)
				{
					StopListenToDirectory(NewChange.FilePath);
				}
			}

			if (Event->mask & IN_CREATE || Event->mask & IN_MOVED_TO)
			{
				NewChange.Type = FileChangeType::Created;
			}
			if (Event->mask & IN_DELETE || Event->mask & IN_MOVED_FROM)
			{
				NewChange.Type = FileChangeType::Removed;
			}
			if (Event->mask & IN_MODIFY)
			{
				NewChange.Type = FileChangeType::Modified;
			}

			Changes[NewChange.FilePath].emplace_back(std::move(NewChange));
		}
		LastChangeTime = Time;
	}
	else
	{
		Time++;
		if (LastChangeTime + 10 < Time)
		{
			FlushEvents();
		}
	}
#endif
}

void engine::FileWatcher::FlushEvents()
{
	for (auto& i : this->Changes)
	{
		FileChange CombinedChange;
		CombinedChange.FilePath = i.first;

		for (auto& item : i.second)
		{
			if (int32(CombinedChange.Type) > int32(item.Type))
			{
				CombinedChange.Type = item.Type;
			}
		}

		this->OnFileChanged.Invoke(CombinedChange);
	}
	Changes.clear();
}

#if WINDOWS

void engine::FileWatcher::WaitForNextChanges()
{
	if (!ReadDirectoryChangesW(Platform->DirectoryHandle, Platform->ChangeBuffer, sizeof(Platform->ChangeBuffer),
		true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME
		| FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
		nullptr, &Platform->OverlappedWriteData, nullptr))
	{
		Log::Error(platform::GetLastErrorString());
	}
}
#else

void engine::FileWatcher::ListenToDirectory(string Name)
{
	Name = std::filesystem::relative(Name);

	int NewFile = inotify_add_watch(Platform->Main.INotifyFileDescriptor, Name.c_str(),
		IN_DELETE | IN_MOVE | IN_MODIFY | IN_CREATE);

	Platform->Main.DescriptorMap[NewFile] = Name;
}

void engine::FileWatcher::StopListenToDirectory(string Name)
{
	for (auto it = Platform->Main.DescriptorMap.begin(); it != Platform->Main.DescriptorMap.end(); it++)
	{
		if (it->second == Name)
		{
			inotify_rm_watch(Platform->Main.INotifyFileDescriptor, it->first);
			Platform->Main.DescriptorMap.erase(it);

		}
	}
}

#endif
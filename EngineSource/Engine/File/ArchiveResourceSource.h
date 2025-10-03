#pragma once
#include "Resource.h"
#include <mutex>
#include <Core/Archive/Archive.h>

namespace engine::resource
{
	class ArchiveResourceSource : public ResourceSource
	{
	public:

		ArchiveResourceSource();

		bool FileExists(string Path) override;
		ReadOnlyBufferStream* GetFile(string Path) override;
		std::map<string, string> GetFiles() override;
		void LoadSceneFiles(string ScenePath) override;

		bool UseArchives = false;

		std::mutex ArchiveMutex;
		std::map<string, Archive*> LoadedArchives;
		std::map<string, std::vector<string>> SceneArchives;
		std::map<string, string> ArchiveAssets;

		void LoadArchive(string Name);

		void UnloadArchive(string Name);
	};
}
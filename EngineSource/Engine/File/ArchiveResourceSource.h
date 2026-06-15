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
		IBinaryStream* GetFile(string Path) override;
		std::map<string, string> GetFiles() override;
		void LoadSceneFiles(string ScenePath) override;

		std::mutex ArchiveMutex;
		std::map<string, Archive*> LoadedArchives;
		std::map<string, std::vector<string>> SceneArchives;
		std::map<string, string> ArchiveAssets;
		// Maps file paths to their archives.
		std::map<string, string> FileMap;

		void LoadArchive(string Name);

		void UnloadArchive(string Name);

	private:

		string ArchiveNameToPath(const string& Name)
		{
			return "Assets/" + Name + ".bin";
		}
	};
}
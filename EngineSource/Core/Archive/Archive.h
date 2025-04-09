#pragma once
#include <Core/Types.h>
#include <Core/File/BinaryStream.h>
#include <map>

namespace engine
{

	class Archive
	{
	public:
		Archive();
		Archive(string FileName);
		Archive(const Archive&) = delete;
		~Archive();

		ReadOnlyBufferStream* GetFile(string Name) const;

		void AddFile(string Name, IBinaryStream* Stream);
		bool HasFile(string Name);

		string ConvertFileName(string Name) const;

		std::vector<string> GetArchiveFiles();

		void Save(string FilePath);
	private:
		struct ArchiveData
		{
			uByte* Bytes = nullptr;
			size_t Size = 0;
		};

		std::map<string, ArchiveData> Streams;
		std::map<string, string> FileNames;
	};
}
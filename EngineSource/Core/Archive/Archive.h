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
		Archive(IBinaryStream* FromStream);
		~Archive();

		ReadOnlyBufferStream* GetFile(string Name) const;

		void AddFile(string Name, IBinaryStream* Stream);
		bool HasFile(string Name);

		string ConvertFileName(string Name) const;

		std::vector<string> GetArchiveFiles();

		void Save(string FilePath);
		void Save(IBinaryStream* Stream);
	private:

		void LoadInternal(IBinaryStream* Stream);
		Archive(const Archive&) = default;

		struct ArchiveData
		{
			uByte* Bytes = nullptr;
			size_t Size = 0;
		};

		std::map<string, ArchiveData> Streams;
		std::map<string, string> FileNames;
	};
}
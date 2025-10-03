#pragma once
#include <Core/Types.h>
#include <Core/Event.h>
#include <Core/File/BinaryStream.h>
#include <vector>

namespace engine::editor
{
	struct AssetFile
	{
		string Path;
		bool IsDirectory;
	};

	class AssetListProvider
	{
	public:
		virtual ~AssetListProvider() = default;
		virtual std::vector<AssetFile> GetFiles(string Path) = 0;
		virtual void DeleteFile(string Path) = 0;
		virtual void NewFile(string Path) = 0;
		virtual void NewDirectory(string Path) = 0;
		virtual IBinaryStream* GetFileSaveStream(string Path) = 0;
		virtual void SaveToFile(string Path, IBinaryStream* Stream, size_t Length)
		{
			auto OutStream = GetFileSaveStream(Path);

			uByte Buffer[2048];
			size_t ToWrite = Length;
			while (ToWrite > 0)
			{
				size_t ChunkSize = std::min(sizeof(Buffer), ToWrite);
				Stream->Read(Buffer, ChunkSize);
				OutStream->Write(Buffer, ChunkSize);
			}
			delete OutStream;
		}

		Event<> OnChanged;
	};
}
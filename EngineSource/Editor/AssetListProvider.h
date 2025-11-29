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
		bool IsDirectory = false;
	};
	/**
	 * @brief
	 * Provides a list of assets and functions to manipulate them to the editor, working like a filesystem.
	 */
	class AssetListProvider
	{
	public:
		virtual ~AssetListProvider() = default;

		/**
		 * @brief
		 * Gets all assets the provider has access to.
		 * @param Path
		 * The path where the provider should search.
		 * @return
		 * A list of assets at the given path.
		 */
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
				ToWrite -= ChunkSize;
			}
			delete OutStream;
		}

		/**
		 * @brief
		 * Called when the files the provider has access to have changed.
		 */
		Event<> OnChanged;
	};
}
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
		/// Virtual destructor.
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

		/**
		 * @brief
		 * Removes a file or directory at the given location.
		 *
		 * This will call @ref OnChanged once the operation is done.
		 * @param Path
		 * The path to remove.
		 */
		virtual void DeleteFile(string Path) = 0;
		/**
		 * @brief
		 * Creates a new empty file at the given location.
		 *
		 * This will call @ref OnChanged once the operation is done.
		 * @param Path
		 * The path of the new file.
		 */
		virtual void NewFile(string Path) = 0;
		/**
		 * @brief
		 * Creates a new directory with the given path.
		 *
		 * This will call @ref OnChanged once the operation is done.
		 * @param Path
		 * The path that the new directory should be created at.
		 */
		virtual void NewDirectory(string Path) = 0;
		/**
		 * @brief
		 * Tries to get a stream that can be written.
		 *
		 * If the stream cannot be acquired, use @ref SaveToFile() to write the data instead.
		 *
		 * @param Path
		 * The path of the file to write to.
		 * @return
		 * A binary stream to write to if that's possible. nullptr otherwise.
		 */
		virtual IBinaryStream* GetFileSaveStream(string Path) = 0;
		/**
		 * @brief
		 * Saves the given stream to a file.
		 * @param Path
		 * The path to the file that should be saved.
		 * @param Stream
		 * The stream containing data to save.
		 * @param Length
		 * The amount of data in bytes to save.
		 */
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
#include "Archive.h"
#include "miniz.h"
#include <Core/File/FileUtil.h>
#include <Core/Log.h>

using namespace engine;

const string ARCHIVE_FORMAT_NAME = "k2arch";
const string ARCHIVE_FORMAT_VERSION = "0";
const string ARCHIVE_FORMAT_STRING = ARCHIVE_FORMAT_NAME + "/" + ARCHIVE_FORMAT_VERSION;

engine::Archive::Archive()
{
}

engine::Archive::Archive(string FileName)
{
	FileStream Stream = FileStream(FileName, true);
	LoadInternal(&Stream);
}

engine::Archive::Archive(IBinaryStream* FromStream)
{
	LoadInternal(FromStream);
}

engine::Archive::~Archive()
{
	for (auto& i : Streams)
	{
		delete i.second.Bytes;
	}
}

engine::ReadOnlyBufferStream* engine::Archive::GetFile(string Name) const
{
	auto Found = Streams.find(ConvertFileName(Name));
	if (Found != Streams.end())
	{
		return new ReadOnlyBufferStream(Found->second.Bytes, Found->second.Size, false);
	}
	return nullptr;
}

void engine::Archive::AddFile(string Name, IBinaryStream* Stream)
{
	size_t NewEntrySize = Stream->GetSize();
	uByte* NewEntryBytes = new uByte[NewEntrySize]();
	Stream->Read(NewEntryBytes, NewEntrySize);

	Streams.insert({ Name, ArchiveData{
		.Bytes = NewEntryBytes,
		.Size = NewEntrySize
		} });
	FileNames.insert({ file::FileName(Name), Name });
}

bool engine::Archive::HasFile(string Name)
{
	return Streams.contains(ConvertFileName(Name));
}

string engine::Archive::ConvertFileName(string Name) const
{
	auto Found = FileNames.find(Name);
	if (Found != FileNames.end())
	{
		return Found->second;
	}
	return Name;
}

std::vector<string> engine::Archive::GetArchiveFiles()
{
	std::vector<string> Found;

	for (auto& i : this->FileNames)
	{
		Found.push_back(i.first);
	}

	return Found;
}

void engine::Archive::Save(string FilePath)
{
	FileStream ArchiveFile = FileStream(FilePath, false);

	Save(&ArchiveFile);
}

void engine::Archive::Save(IBinaryStream* Stream)
{
	// Writes file header: [format string (string)] [file count (size_t)]

	Stream->WriteString(ARCHIVE_FORMAT_STRING);

	Stream->WriteValue(Streams.size());

	for (auto& [Name, Data] : Streams)
	{
		// Writes file entry: [name (string)] [compressed size (size_t)] [uncompressed size (size_t)] [file buffer (uByte * compressed size)]

		Stream->WriteString(Name);

		if (Data.Size > 0)
		{
			uByte* CompressedFile = new uByte[Data.Size]();

			mz_ulong CompressedSize = mz_ulong(Data.Size);
			int Error = mz_compress(CompressedFile, &CompressedSize, Data.Bytes, mz_ulong(Data.Size));

			if (Error != MZ_OK)
			{
				Log::Warn("Deflate compress error: " + string(mz_error(Error)) + ". Storing uncompressed.");
				Stream->WriteValue(Data.Size);
				Stream->WriteValue(Data.Size);
				Stream->Write(Data.Bytes, Data.Size);
			}
			else
			{
				Stream->WriteValue(size_t(CompressedSize));
				Stream->WriteValue(Data.Size);

				Stream->Write(CompressedFile, CompressedSize);
				delete[] CompressedFile;
			}
		}
		else
		{
			Stream->WriteValue(size_t(0)); // Compressed size
			Stream->WriteValue(size_t(0)); // Uncompressed size
		}
	}
}

void engine::Archive::LoadInternal(IBinaryStream* Stream)
{
	// Reads file header: [format string (string)] [file count (size_t)]

	string FileHeader;
	Stream->ReadString(FileHeader);
	if (FileHeader != ARCHIVE_FORMAT_STRING)
	{
		Log::Error("Format error. Incorrect header.");
		return;
	}

	size_t NumFiles = Stream->Get<size_t>();

	for (size_t i = 0; i < NumFiles; i++)
	{
		// Reads file entry: [name (string)] [compressed size (size_t)] [uncompressed size (size_t)] [file buffer (uByte * compressed size)]

		string FileName;
		Stream->ReadString(FileName);
		size_t CompressedSize = Stream->Get<size_t>();
		mz_ulong FileSize = mz_ulong(Stream->Get<size_t>());

		if (!CompressedSize)
		{
			Streams.insert({ FileName, ArchiveData{
				.Bytes = new Byte[0],
				.Size = 0
				} });
			continue;
		}

		uByte* CompressedFile = new uByte[CompressedSize];

		if (!Stream->Read(CompressedFile, CompressedSize))
		{
			Log::Error(str::Format("Format error. Could not read %i bytes of file %s", CompressedSize, FileName.c_str()));
			delete[] CompressedFile;
			return;
		}

		// No compression
		if (FileSize == CompressedSize)
		{
			Streams.insert({ FileName, ArchiveData{
				.Bytes = CompressedFile,
				.Size = FileSize
				} });

			FileNames.insert({ file::FileName(FileName), FileName });
		}
		else
		{
			uByte* DecompressedFile = new uByte[FileSize];

			int Error = mz_uncompress(DecompressedFile, &FileSize, CompressedFile, mz_ulong(CompressedSize));

			if (Error != MZ_OK)
			{
				Log::Error("Deflate compress error: " + string(mz_error(Error)));
				break;
			}

			delete[] CompressedFile;

			Streams.insert({ FileName, ArchiveData{
				.Bytes = DecompressedFile,
				.Size = FileSize
				} });

			FileNames.insert({ file::FileName(FileName), FileName });
		}
	}
}
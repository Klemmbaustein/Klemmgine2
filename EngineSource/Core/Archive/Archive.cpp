#include "Archive.h"
#include "miniz.h"
#include <Core/File/FileUtil.h>

using namespace engine;

const string ARCHIVE_FORMAT_NAME = "k2arch";
const string ARCHIVE_FORMAT_VERSION = "0";
const string ARCHIVE_FORMAT_STRING = ARCHIVE_FORMAT_NAME + "/" + ARCHIVE_FORMAT_VERSION;

engine::Archive::Archive()
{
}

engine::Archive::Archive(string FileName)
{
	FileStream ArchiveFile = FileStream(FileName, true);

	// Reads file header: [format string (string)] [file count (size_t)]

	string FileHeader;
	ArchiveFile.ReadString(FileHeader);
	if (FileHeader != ARCHIVE_FORMAT_STRING)
	{
		return;
	}

	size_t NumFiles = ArchiveFile.Get<size_t>();

	for (size_t i = 0; i < NumFiles; i++)
	{
		// Reads file entry: [name (string)] [compressed size (size_t)] [uncompressed size (size_t)] [file buffer (uByte * compressed size)]

		string FileName;
		ArchiveFile.ReadString(FileName);
		size_t CompressedSize = ArchiveFile.Get<size_t>();
		mz_ulong FileSize = mz_ulong(ArchiveFile.Get<size_t>());

		uByte* CompressedFile = new uByte[CompressedSize]();
		uByte* DecompressedFile = new uByte[FileSize]();
		ArchiveFile.Read(CompressedFile, CompressedSize);

		mz_uncompress(DecompressedFile, &FileSize, CompressedFile, mz_ulong(CompressedSize));

		delete[] CompressedFile;

		Streams.insert({ FileName, ArchiveData{
			.Bytes = DecompressedFile,
			.Size = FileSize
			} });

		FileNames.insert({ file::FileName(FileName), FileName });
	}
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

	// Writes file header: [format string (string)] [file count (size_t)]

	ArchiveFile.WriteString(ARCHIVE_FORMAT_STRING);

	ArchiveFile.WriteValue(Streams.size());

	for (auto& [Name, Data] : Streams)
	{
		// Writes file entry: [name (string)] [compressed size (size_t)] [uncompressed size (size_t)] [file buffer (uByte * compressed size)]

		ArchiveFile.WriteString(Name);

		uByte* CompressedFile = new uByte[Data.Size]();

		mz_ulong CompressedSize = mz_ulong(Data.Size);
		mz_compress(CompressedFile, &CompressedSize, Data.Bytes, mz_ulong(Data.Size));

		ArchiveFile.WriteValue(size_t(CompressedSize));
		ArchiveFile.WriteValue(Data.Size);

		ArchiveFile.Write(CompressedFile, CompressedSize);

		delete[] CompressedFile;
	}
}

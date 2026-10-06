#include "EngineUIResource.h"
#include <Engine/File/AssetRef.h>
#include <Engine/File/Resource.h>

using namespace kui;
using namespace kui::resource;

bool engine::EngineUIResource::FileExists(const std::string& Path)
{
	return Path.substr(0, 4) != "res:" && AssetRef::Convert(Path).Exists();
}

BinaryData engine::EngineUIResource::GetFile(const std::string& File)
{
	auto Found = resource::GetBinaryFile(AssetRef::Convert(File).FilePath);

	size_t Length = Found->GetSize();

	if (Length)
	{
		uByte* AllData = new uByte[Length];
		Found->Read(AllData, Length);
		delete Found;

		return BinaryData{
			.Data = AllData,
			.FileSize = Length
		};
	}
	delete Found;
	return BinaryData();
}

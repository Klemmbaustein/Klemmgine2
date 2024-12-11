#include "AssetRef.h"
#include "FileUtil.h"

engine::AssetRef operator""_asset(const char* Name, std::size_t Size)
{
	engine::string Out = engine::string(Name, Size);

	size_t Dot = Out.find_last_of(".");

	return engine::AssetRef::FromName(Out.substr(0, Dot), Out.substr(Dot + 1));
}

engine::AssetRef engine::AssetRef::FromName(string Name, string Extension)
{
	return AssetRef{
		.FilePath = Name,
		.Extension = Extension,
	};
}

engine::AssetRef engine::AssetRef::FromPath(string Path)
{
	size_t Dot = Path.find_last_of(".");

	return AssetRef{
		.FilePath = Path,
		.Extension = Path.substr(Dot + 1),
	};
}

engine::string engine::AssetRef::DisplayName() const
{
	return file::FileNameWithoutExt(FilePath);
}

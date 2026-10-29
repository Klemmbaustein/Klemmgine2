#include "AssetRef.h"
#include "Resource.h"
#include <Core/File/FileUtil.h>

using namespace engine;

engine::AssetRef operator""_asset(const char* Name, std::size_t Size)
{
	string Out = engine::string(Name, Size);

	size_t Dot = Out.find_last_of(".");

	return AssetRef::FromName(Out.substr(0, Dot), Out.substr(Dot + 1));
}

engine::AssetRef engine::AssetRef::FromName(string Name, string Extension)
{
	string FullName = Name + "." + Extension;

	auto Found = resource::LoadedAssets.find(FullName);

	if (Found == resource::LoadedAssets.end())
	{
		return AssetRef(Name.empty() ? "" : FullName, Extension, std::nullopt);
	}

	return AssetRef(Found->second, Extension, true);
}

AssetRef engine::AssetRef::EmptyAsset(string Extension)
{
	return AssetRef(Extension, false);
}

engine::AssetRef engine::AssetRef::FromPath(string Path)
{
	size_t Dot = Path.find_last_of(".");

	return AssetRef(Path, Path.substr(Dot + 1), std::nullopt);
}

engine::AssetRef engine::AssetRef::Convert(std::string PathOrName)
{
	if (resource::FileExists(PathOrName))
	{
		return AssetRef::FromPath(PathOrName);
	}
	size_t Dot = PathOrName.find_last_of(".");

	return engine::AssetRef::FromName(PathOrName.substr(0, Dot), PathOrName.substr(Dot + 1));
}

bool engine::AssetRef::IsValid() const
{
	return !FilePath.empty();
}

engine::string engine::AssetRef::DisplayName() const
{
	return file::FileNameWithoutExt(FilePath);
}

bool engine::AssetRef::Exists() const
{
	if (ExistsCache)
	{
		return *ExistsCache;
	}

	bool Result = resource::FileExists(FilePath);

	ExistsCache = Result;

	return Result;
}

engine::AssetRef::AssetRef(string Path, string Extension, std::optional<bool> ExistsCache)
	: FilePath(Path), Extension(Extension), ExistsCache(ExistsCache)
{
}

engine::AssetRef::AssetRef(string Extension, std::optional<bool> ExistsCache)
	: Extension(Extension), ExistsCache(ExistsCache)
{
}

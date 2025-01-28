#include "AssetRef.h"
#include "Resource.h"
#include "FileUtil.h"
#include <Engine/Log.h>
#include <filesystem>

engine::AssetRef operator""_asset(const char* Name, std::size_t Size)
{
	engine::string Out = engine::string(Name, Size);

	size_t Dot = Out.find_last_of(".");

	return engine::AssetRef::FromName(Out.substr(0, Dot), Out.substr(Dot + 1));
}

engine::AssetRef engine::AssetRef::FromName(string Name, string Extension)
{
	string FullName = Name + "." + Extension;

	auto Found = resource::LoadedAssets.find(FullName);

	if (Found == resource::LoadedAssets.end())
	{
		return AssetRef{
			.FilePath = "",
			.Extension = Extension
		};
	}

	return AssetRef{
		.FilePath = Found->second,
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

engine::AssetRef engine::AssetRef::Convert(std::string PathOrName)
{
	if (std::filesystem::exists(PathOrName))
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

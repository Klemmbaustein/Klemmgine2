#include "ToArchives.h"
#include <Core/File/TextSerializer.h>
#include <Core/File/BinarySerializer.h>
#include <fstream>
#include <Core/Log.h>
#include <map>

using namespace engine::build;

std::vector<ArchiveInfo> engine::build::GetBuildArchives(fs::path AssetsPath)
{
	std::vector<fs::path> FoundScenes;
	std::map<fs::path, AssetDependency> FoundFiles;
	std::map<string, fs::path> FileNames;

	for (auto& file : fs::recursive_directory_iterator(AssetsPath))
	{
		string Extension = file.path().extension().string();

		if (Extension == ".kts")
		{
			FoundScenes.push_back(file);
		}
		else if (fs::is_regular_file(file))
		{
			FoundFiles.insert({ file, AssetDependency{
				.File = file,
				.DependentScenes = {}
			} });
			FileNames.insert({file.path().filename().string(), file.path()});
		}
	}

	auto GetFileFromName =
		[&FileNames](string Name) -> fs::path
		{
			if (fs::exists(Name))
				return fs::path(Name);
			return FileNames[Name];
		};

	std::function<void(fs::path, fs::path)> ReadDependencies =
		[&FoundFiles, &ReadDependencies, &GetFileFromName](fs::path Asset, fs::path RootDependency)
		{
			auto Dependencies = GetFileDependencies(Asset, GetFileFromName);

			for (auto& dep : Dependencies)
			{
				FoundFiles[dep].DependentScenes.insert(RootDependency);
				ReadDependencies(dep, RootDependency);
			}
		};

	for (auto& scn : FoundScenes)
	{
		ReadDependencies(scn, scn);
	}

	std::map<string, ArchiveInfo> OutArchives;

	for (auto& i : FoundFiles)
	{
		string ArchiveIdentifier;
		for (auto& scn : i.second.DependentScenes)
		{
			ArchiveIdentifier += scn.string() + ";";
		}

		auto& Arch = OutArchives[ArchiveIdentifier];
		Arch.Files.insert(i.first);
		for (auto& scn : i.second.DependentScenes)
		{
			Arch.DependentScenes.insert(scn);
		}
	}

	std::vector<ArchiveInfo> Out;
	Out.reserve(OutArchives.size());

	int Counter = 0;
	for (auto& i : OutArchives)
	{
		i.second.Name = str::Format("assets%i", Counter++);
		Out.push_back(i.second);
	}

	Out.push_back(ArchiveInfo{
		.Name = "scenes",
		.Files = std::set(FoundScenes.begin(), FoundScenes.end()),
		});

	return Out;
}

std::set<fs::path> engine::build::GetFileDependencies(fs::path FilePath, std::function<fs::path(string)> GetFileFromName)
{
	std::set<fs::path> out;
	if (FilePath.extension() == ".kts")
	{
		try
		{
			std::fstream SceneFile = std::fstream(FilePath, std::ios::in);
			SerializedValue Scene = TextSerializer::FromStream(SceneFile);

			auto& Objects = Scene.At("objects").GetArray();
			for (SerializedValue& obj : Objects)
			{
				auto& Properties = obj.At("properties").GetObject();
				for (auto& property : Properties)
				{
					string Path = property.Value.GetString();
					if (fs::exists(Path))
						out.insert(fs::path(Path));
				}
			}
		}
		catch (SerializeReadException e)
		{
			Log::Warn(str::Format("%s: %s", FilePath.c_str(), e.what()));
		}
	}
	if (FilePath.extension() == ".kmdl")
	{
		SerializedValue Model = BinarySerializer::FromFile(FilePath.string(), "kmdl");
		auto& Meshes = Model.At("meshes").At("meshes").GetArray();
		for (auto& i : Meshes)
		{
			fs::path Material = GetFileFromName(i.At("mat").GetString() + ".kmt");
			if (fs::exists(Material))
				out.insert(Material);
		}
	}
	if (FilePath.extension() == ".kmt")
	{
		std::fstream SceneFile = std::fstream(FilePath, std::ios::in);
		SerializedValue Material = TextSerializer::FromStream(SceneFile);
		for (auto& i : Material.GetObject())
		{
			if (i.Value.GetType() != SerializedData::DataType::Object)
				continue;
			if (i.At("type").GetInt() != 4)
				continue;

			if (!i.Value.Contains("val"))
				continue;

			auto& TextureValue = i.At("val");
			fs::path Texture;

			if (TextureValue.GetType() == SerializedData::DataType::String)
			{
				Texture = GetFileFromName(TextureValue.GetString());
			}
			else
			{
				Texture = GetFileFromName(TextureValue.At("file").GetString());
			}
			if (fs::exists(Texture))
				out.insert(Texture);
		}
	}

	return out;
}

#include <Core/Types.h>
#include <Core/Log.h>
#include <Core/File/FileUtil.h>
#include <Core/File/BinarySerializer.h>
#include <Core/File/TextSerializer.h>
#include <Core/LaunchArgs.h>
#include <Core/Archive/Archive.h>
#include <Core/ThreadPool.h>
#include <filesystem>
#include <map>
#include "ToArchives.h"
#include "Plugins.h"

using namespace engine;
namespace std
{
	namespace fs = std::filesystem;
}

void CopyBinaries(std::fs::path BinaryPath, std::fs::path OutPath)
{
	static std::vector<string> Executables = {
		"Klemmgine"
	};
	static std::vector<string> SharedLibraries = {
		"SDL3",
		"OpenAL32"
	};

	bool WithDebugInfo = launchArgs::GetArg("includePdb").has_value();

	for (string i : Executables)
	{
#if WINDOWS
		if (WithDebugInfo)
		{
			string PdbName = i + ".pdb";

			std::fs::path DebugPath = BinaryPath / PdbName;

			if (!std::fs::exists(DebugPath))
			{
				Log::Warn(str::Format("-includePdb is set, but the pdb for the file %s.exe doesn't exist!", i.c_str()));
			}
			else
			{
				std::fs::copy(DebugPath, OutPath / PdbName);
			}
		}
		i.append(".exe");
#endif
		std::fs::copy(BinaryPath / i, OutPath / i);
	}
#if WINDOWS
	for (string i : SharedLibraries)
	{
		i.append(".dll");
		if (std::fs::exists(BinaryPath / i))
			std::fs::copy(BinaryPath / i, OutPath / i);
	}
#endif
}

static BufferStream* CopyAndConvertFile(std::fs::path InPath, string& OutPath)
{
	std::map<string, string> KnownExtensions = {
		{"kmt", "kbm"},
		{"kts", "kbs"},
	};

	auto Name = InPath.filename();

	string NameStr = InPath.string();

	size_t Dot = NameStr.find_last_of(".");

	if (Dot != string::npos)
	{
		string Extension = NameStr.substr(Dot + 1);
		string WithoutExtension = NameStr.substr(0, Dot);

		if (KnownExtensions.contains(Extension))
		{
			string NewExtension = KnownExtensions[Extension];
			OutPath = str::ReplaceChar(WithoutExtension, '\\', '/') + "." + NewExtension;

			Log::Note(str::Format("Convert to binary: %s -> %s",
				str::ReplaceChar(InPath.string(), '\\', '/').c_str(),
				OutPath.c_str()));

			BufferStream* NewStream = new BufferStream();

			BinarySerializer::ToBinaryData(
				TextSerializer::FromFile(InPath.string()),
				NewStream,
				NewExtension
			);
			NewStream->ResetStreamPosition();
			return NewStream;
		}
	}
	return nullptr;
}

int main(int argc, char** argv)
{
	launchArgs::SetArgs(argc, argv);
	Log::IsVerbose = launchArgs::GetArg("verbose").has_value();

	string OutPath = "build/dev";

	auto OutputArg = launchArgs::GetArg("out");
	if (OutputArg && OutputArg->size() == 1)
	{
		OutPath = OutputArg->at(0).AsString();
	}
	else
	{
		Log::Warn("No output path specified using -out. Using default: build/");
	}

	string BinPath = file::FilePath(argv[0]);
	auto BinArg = launchArgs::GetArg("binPath");
	if (BinArg && BinArg->size() == 1)
	{
		BinPath = BinArg->at(0).AsString();
	}

	try
	{
		if (std::fs::exists(OutPath))
		{
			std::fs::remove_all(OutPath);
		}
	}
	catch (std::fs::filesystem_error e)
	{
		Log::Error(e.what());
		return 1;
	}

	std::fs::create_directories(std::fs::path(OutPath) / "Assets");
	Log::Info("Building into " + std::fs::canonical(OutPath).string() + " from " + std::fs::canonical(".").string());

	try
	{
		ThreadPool ArchiveThreads = engine::ThreadPool(std::thread::hardware_concurrency() - 1, "Archive workers");

		build::CopyPluginFiles(BinPath, OutPath);
		CopyBinaries(BinPath, OutPath);

		auto Archives = engine::build::GetBuildArchives("Assets/");
		Log::Info(str::Format("Starting %i archive compression jobs...", int(Archives.size())));

		std::atomic<uint32> WrittenArchives;

		SerializedValue MapFile = std::vector<SerializedData>();
		for (auto& ArchiveFiles : Archives)
		{
			std::fs::path ResultPath = std::fs::relative(std::fs::path(OutPath) / "Assets" / (ArchiveFiles.Name + ".bin"));
			ArchiveThreads.AddJob([ArchiveFiles, ResultPath, ArchiveSize = Archives.size(), &WrittenArchives]() {
				Log::Info("Writing archive: "
					+ ResultPath.string()
					+ str::Format(" (%i/%i)", uint32(++WrittenArchives), uint32(ArchiveSize)));

				Archive a;

				for (auto& file : ArchiveFiles.Files)
				{
					string Name = str::ReplaceChar(file.string(), '\\', '/');

					IBinaryStream* Data = CopyAndConvertFile(file, Name);
					if (!Data)
					{
						Data = new FileStream(file.string(), true);
					}

					a.AddFile(Name, Data);
					delete Data;
				}
				a.Save(ResultPath.string());
				Log::Note("Done writing archive: " + ResultPath.string());
				});

			SerializedValue SceneArray = std::vector<SerializedValue>();

			for (auto& i : ArchiveFiles.DependentScenes)
			{
				string SceneString = str::ReplaceChar(i.string(), '\\', '/');
				SceneString = file::FilePath(SceneString) + "/" + file::FileNameWithoutExt(SceneString) + ".kbs";

				SceneArray.Append(SceneString);
			}

			MapFile.Append(SerializedData(ArchiveFiles.Name, SceneArray));
		}

		BinarySerializer::ToFile(MapFile.GetObject(), OutPath + "/Assets/archmap.bin", "archm");
	}
	catch (std::fs::filesystem_error e)
	{
		Log::Error(e.what());
		return 2;
	}

	return 0;
}
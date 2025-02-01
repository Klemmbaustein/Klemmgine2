#include <Core/Types.h>
#include <Core/Log.h>
#include <Core/File/FileUtil.h>
#include <Core/File/BinarySerializer.h>
#include <Core/File/TextSerializer.h>
#include <Core/LaunchArgs.h>
#include <filesystem>
#include <map>

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
		"nethost",
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
				std::filesystem::copy(DebugPath, OutPath / PdbName);
			}
		}
		i.append(".exe");
#endif
		std::filesystem::copy(BinaryPath / i , OutPath / i);
	}
	for (string i : SharedLibraries)
	{
#if WINDOWS
		i.append(".dll");
#elif LINUX
		i = str::Format("lib%s.so", i.c_str());
#endif
		std::filesystem::copy(BinaryPath / i, OutPath / i);
	}
}

void CopyAndConvertFiles(std::fs::path InPath, std::fs::path OutPath)
{
	std::map<string, string> KnownExtensions = {
		{"kmt", "kbm"},
		{"kts", "kbs"},
	};

	for (auto& i : std::fs::directory_iterator(InPath))
	{
		auto Name = i.path().filename();

		if (std::fs::is_directory(i))
		{
			std::fs::create_directory(OutPath / Name);
			CopyAndConvertFiles(InPath / Name, OutPath / Name);
			continue;
		}

		string NameStr = Name.string();

		size_t Dot = NameStr.find_last_of(".");
		if (Dot != string::npos)
		{
			string Extension = NameStr.substr(Dot + 1);
			string WithoutExtension = NameStr.substr(0, Dot);

			if (KnownExtensions.contains(Extension))
			{
				string NewExtension = KnownExtensions[Extension];
				std::string NewFile = (OutPath / WithoutExtension).string() + "." + NewExtension;

				Log::Info(str::Format("Convert to binary: %s -> %s",
					str::ReplaceChar(i.path().string(), '\\', '/').c_str(),
					str::ReplaceChar(NewFile, '\\', '/').c_str()));

				BinarySerializer::ToFile(
					TextSerializer::FromFile(i.path().string()),
					NewFile,
					NewExtension
				);
				continue;
			}
		}
		
		std::fs::copy(i, OutPath / i.path().filename());
	}
}

int main(int argc, char** argv)
{
	launchArgs::SetArgs(argc, argv);

	string OutPath = "build";

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
	Log::Info("Building into " + OutPath);

	try
	{
		CopyAndConvertFiles("Assets/", OutPath + "/Assets");

		CopyBinaries(BinPath, OutPath);
	}
	catch (std::fs::filesystem_error e)
	{
		Log::Error(e.what());
	}

	return 0;
}
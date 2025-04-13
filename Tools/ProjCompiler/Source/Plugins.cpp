#include "Plugins.h"
#include <Core/File/TextSerializer.h>
#include <Core/Log.h>
#include <Core/File/FileUtil.h>
#include <Core/LaunchArgs.h>
using namespace engine;

#if WINDOWS
static const string DefaultPlatform = "win-x64";
#elif LINUX
static const string DefaultPlatform = "linux-x64";
#endif

static SerializedValue GetPlatformValue(const SerializedValue& Value)
{
	string Platform = DefaultPlatform;
	auto PlatformArg = launchArgs::GetArg("platform");

	if (PlatformArg)
	{
		Platform = PlatformArg.value().at(0).AsString();
	}

	if (Value.GetType() != SerializedData::DataType::Object)
	{
		return Value;
	}
	for (auto& PlatformDependent : Value.GetObject())
	{
		if (PlatformDependent.Name == Platform)
		{
			return PlatformDependent.Value;
		}
	}
	return SerializedValue();
}

void engine::build::CopyPluginFiles(fs::path BinaryPath, fs::path OutPath)
{
	std::vector<fs::path> FoundPlugins;

	for (auto& dir : fs::directory_iterator("Plugins"))
	{
		if (fs::is_regular_file(dir.path() / "Plugin.k2p"))
		{
			FoundPlugins.push_back(dir);
		}
	}

	bool IncludeDev = launchArgs::GetArg("devBuild").has_value();

	for (auto& p : FoundPlugins)
	{
		try
		{
			auto PluginBinPath = OutPath / "Plugins" / "bin";
			auto PluginOutPath = OutPath / "Plugins" / p.filename();
			auto PluginFile = SerializedValue(TextSerializer::FromFile((p / "Plugin.k2p").string()));

			if (PluginFile.Contains("devOnly") && PluginFile.At("devOnly").GetBool() && !IncludeDev)
			{
				Log::Info(str::Format("Skipping plugin %s - marked as devOnly", p.filename().string().c_str()));
				continue;
			}

			Log::Info(str::Format("Copying plugin %s", p.filename().string().c_str()));
			std::filesystem::create_directories(PluginBinPath);

			std::filesystem::create_directories(PluginOutPath);
			if (PluginFile.Contains("include"))
			{
				auto CopyStringFile = [&p, &BinaryPath, &PluginBinPath](string FileName) -> bool {

					fs::path PathToTest = BinaryPath / "plugins" / FileName;

					if (fs::exists(PathToTest))
					{
						fs::copy(PathToTest, PluginBinPath / file::FileName(FileName));
						return true;
					}

					PathToTest = p / FileName;
					if (fs::exists(PathToTest))
					{
						fs::copy(PathToTest, PluginBinPath / file::FileName(FileName));
						return true;
					}
					return false;
					};

				for (auto& i : PluginFile.At("include").GetArray())
				{
					auto ToInclude = GetPlatformValue(i);

					CopyStringFile(ToInclude.GetString());
				}
			}

			string PluginBinary = PluginFile.At("binary").GetString();

#if WINDOWS
			PluginBinary = str::Format("%s.dll", PluginBinary.c_str());
#else
			PluginBinary = str::Format("lib%s.so", PluginBinary.c_str());
#endif

			fs::copy(BinaryPath / "plugins" / PluginBinary, PluginBinPath / PluginBinary);
			fs::copy(p / "Plugin.k2p", PluginOutPath / "Plugin.k2p");
		}
		catch (SerializeException& e)
		{
			Log::Warn(e.what());
		}
		catch (std::exception& e)
		{
			Log::Warn(e.what());
		}
	}
}

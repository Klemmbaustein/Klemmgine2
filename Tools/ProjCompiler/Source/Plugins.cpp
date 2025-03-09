#include "Plugins.h"
#include <Core/File/TextSerializer.h>
#include <Core/Log.h>
#include <Core/File/FileUtil.h>

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

	for (auto& p : FoundPlugins)
	{
		try
		{
			auto PluginOutPath = OutPath / "Plugins" / p.filename();
			auto PluginFile = SerializedValue(TextSerializer::FromFile((p / "Plugin.k2p").string()));

			if (PluginFile.Contains("devOnly") && PluginFile.At("devOnly").GetBool())
			{
				Log::Info(str::Format("Skipping plugin %s - marked as devOnly", p.filename().string().c_str()));
				continue;
			}

			Log::Info(str::Format("Copying plugin %s", p.filename().string().c_str()));

			std::filesystem::create_directories(PluginOutPath);
			if (PluginFile.Contains("include"))
			{
				for (auto& i : PluginFile.At("include").GetArray())
				{
					std::filesystem::create_directories(PluginOutPath / file::FilePath(i.GetString()));
					fs::copy(p / i.GetString(), PluginOutPath / i.GetString());
				}
			}

			string PluginBinary = PluginFile.At("binary").GetString();

#if WINDOWS
			PluginBinary = str::Format("%s.dll", PluginBinary.c_str());
#else
			PluginBinary = str::Format("lib%s.so", PluginBinary.c_str());
#endif

			std::filesystem::create_directories(OutPath / "Plugins" / "bin");
			fs::copy(BinaryPath / "plugins" / PluginBinary, OutPath / "Plugins" / "bin" / PluginBinary);
			fs::copy(p / "Plugin.k2p", PluginOutPath / "Plugin.k2p");
		}
		catch (SerializeException& e)
		{
			Log::Warn(e.what());
		}
	}
}

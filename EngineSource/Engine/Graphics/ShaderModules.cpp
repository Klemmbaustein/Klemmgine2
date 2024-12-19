#include "ShaderModules.h"
#include <sstream>
#include <array>
#include <Engine/Log.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/File/Resource.h>
#include <Engine/Internal/OpenGL.h>
using namespace engine::graphics;

ShaderModuleLoader::ShaderModuleLoader()
{
}

ShaderModuleLoader::~ShaderModuleLoader()
{
}

ShaderModuleLoader::Result ShaderModuleLoader::ParseShader(const string& ShaderSource)
{

	std::stringstream SourceStream;
	SourceStream << ShaderSource;

	std::stringstream OutStream;

	std::vector<ShaderModule> FoundModules;
	// When #export is used, the next line exports a function.
	bool NextLineIsExport = false;
	bool IsModule = false;
	size_t Line = 0;
	ShaderModule ParsedModule;

	auto ParseError = [](string Message) {

		Log::Error(Message, { Log::LogPrefix{
			.Text = "Shader Modules",
			.Color = Log::LogColor::Cyan,
			} });
		};

	auto NextLine = [&OutStream, &Line](string Content) {
		OutStream << Content << std::endl;
		Line++;
		};

	while (true)
	{
		char LineBuffer[8000];
		SourceStream.getline(LineBuffer, sizeof(LineBuffer));

		if (SourceStream.eof())
		{
			NextLine(LineBuffer);
			break;
		}

		string LineString = str::Trim(LineBuffer);

		size_t CommentPos = LineString.find("//");

		if (CommentPos != string::npos)
		{
			LineString = str::Trim(LineString.substr(0, CommentPos));
		}

		if (LineString.empty())
		{
			NextLine("");
			continue;
		}
		if (LineString[0] != '#')
		{
			if (NextLineIsExport)
			{
				if (LineString[LineString.size() - 1] == '{')
					LineString = LineString.substr(0, LineString.size() - 1);

				ParsedModule.Exported.push_back(LineString);
				NextLineIsExport = false;
			}
			NextLine(LineBuffer);
			continue;
		}

		std::vector Statement = str::Split(LineString.substr(1), "\t ");

		if (Statement.empty())
		{
			NextLine("");
			continue;
		}

		if (Statement[0] == "export")
		{
			if (!IsModule)
			{
				ParseError("Can only use #export in modules");
			}

			NextLineIsExport = true;
			NextLine("");
			continue;
		};

		if (Statement[0] == "using")
		{
			if (Statement.size() < 2)
			{
				continue;
			}
			string ModuleFile = Statement[1];

			ShaderModule& Found = LoadedModules[ModuleFile];

			FoundModules.push_back(Found);
		
			for (auto& i : Found.Exported)
			{
				OutStream << i << ";";
			}
			OutStream << "\n#line " << Line + 2 << std::endl;

			NextLine("");
			continue;
		};

		if (Statement[0] == "module")
		{
			IsModule = true;
			ParsedModule.Name = Statement[1];
			NextLine("");
			continue;
		};

		NextLine(LineBuffer);
	}

	return Result{
		.ResultSource = OutStream.str(),
		.DependencyModules = FoundModules,
		.IsModule = IsModule,
		.ThisModule = ParsedModule,
	};
}

void engine::graphics::ShaderModuleLoader::ScanModules()
{
	static std::array<string, 1> EngineDefaultModules =
	{
		"res:shader/engine.common.glsl"
	};

	for (const auto& m : EngineDefaultModules)
	{
		Result Res = ParseShader(resource::GetTextFile(m));

		LoadModule(Res.ResultSource, Res.ThisModule);
		LoadedModules.insert({ Res.ThisModule.Name, Res.ThisModule });
	}
}

void engine::graphics::ShaderModuleLoader::LoadModule(const string& Source, ShaderModule& Info)
{
	Info.ModuleObject = glCreateShader(GL_FRAGMENT_SHADER);
	const char* SourcePointer = Source.c_str();
	glShaderSource(Info.ModuleObject, 1, &SourcePointer, nullptr);
	glCompileShader(Info.ModuleObject);
	ShaderObject::CheckCompileErrors(Info.ModuleObject, "FRAGMENT");
}

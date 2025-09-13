#include "ShaderModules.h"
#include <sstream>
#include <array>
#include <Core/Log.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/File/Resource.h>
#include <Engine/Internal/OpenGL.h>
#include <Engine/Graphics/OpenGL.h>
using namespace engine::graphics;

ShaderModuleLoader::ShaderModuleLoader()
{
}

ShaderModuleLoader::~ShaderModuleLoader()
{
}

ShaderModuleLoader::Result ShaderModuleLoader::ParseShader(const string& ShaderSource, ShaderModule::ShaderType Type)
{
	std::stringstream SourceStream;
	SourceStream << ShaderSource;

	std::stringstream OutStream;

	std::vector<ShaderModule> FoundModules;
	std::vector<ShaderUniform> FoundUniforms;
	// When #export is used, the next line exports a function or variable.
	bool NextLineIsExport = false;
	// When #param is used, the next line exports a GLSL uniform.
	bool NextLineIsParam = false;
	bool IsModule = false;
	size_t Line = 0;
	ShaderModule ParsedModule;
	ParsedModule.Type = Type;

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

	if (openGL::GetGLVersion() >= openGL::Version::GL430)
	{
		OutStream << "#version 430\n";
		OutStream << "#define ENGINE_GL_430 1\n";
		OutStream << "#define ENGINE_GL_330 1\n";
	}
	else
	{
		OutStream << "#version 330\n";
		OutStream << "#define ENGINE_GL_330 1\n";
	}
	OutStream << "#line 1\n";

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
			if (NextLineIsParam)
			{
				FoundUniforms.push_back(ReadUniformDefinition(LineString));
				NextLineIsParam = false;
			}
			NextLine(LineBuffer);
			continue;
		}
		else if (NextLineIsExport || NextLineIsParam)
		{
			return Result();
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

		if (Statement[0] == "param")
		{
			if (IsModule)
			{
				ParseError("Cannot use #param in modules");
			}

			NextLineIsParam = true;
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

			ShaderModule& Found = LoadedModules.at(ModuleFile + (Type == ShaderModule::ShaderType::Vertex ? ":vert" : ":frag"));

			FoundModules.push_back(Found);

			for (auto& i : Found.Exported)
			{
				OutStream << i << ";";
			}
			OutStream << "\n#line " << Line + 1 << std::endl;

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
		.ShaderUniforms = FoundUniforms,
		.IsModule = IsModule,
		.ThisModule = ParsedModule,
	};
}

void engine::graphics::ShaderModuleLoader::ScanModules()
{
	FreeModules();

	static std::array<string, 2> EngineDefaultModules =
	{
		"res:shader/engine.common.vert",
		"res:shader/engine.common.frag",
	};

	for (const auto& m : EngineDefaultModules)
	{
		bool IsVertex = m.substr(m.find_last_of(".") + 1) == "vert";

		auto Type = IsVertex ? ShaderModule::ShaderType::Vertex : ShaderModule::ShaderType::Fragment;

		Result Res = ParseShader(resource::GetTextFile(m), Type);
		LoadModule(Res.ResultSource, Res.ThisModule);
		LoadedModules.insert({ Res.ThisModule.Name + (IsVertex ? ":vert" : ":frag"), Res.ThisModule});
	}
}

void engine::graphics::ShaderModuleLoader::FreeModules()
{
	for (auto& [name, mod] : LoadedModules)
	{
		glDeleteShader(mod.ModuleObject);
	}
	LoadedModules.clear();
}

ShaderUniform engine::graphics::ShaderModuleLoader::ReadUniformDefinition(string DefinitionSource)
{
	DefinitionSource = str::ReplaceChar(DefinitionSource, ';', ' ');
	std::vector SplitValue = str::Split(DefinitionSource, "\t ");

	if (SplitValue.size() < 3)
	{
		return ShaderUniform();
	}

	string Uniform = SplitValue[0];
	string Type = SplitValue[1];
	string Name = SplitValue[2];

	std::map<string, Material::Field::Type> TypeNames = {
		{ "int", Material::Field::Type::Int },
		{ "float", Material::Field::Type::Float },
		{ "sampler2D", Material::Field::Type::Texture },
		{ "vec3", Material::Field::Type::Vec3 },
		{ "bool", Material::Field::Type::Bool },
	};
	string ValueString;
	if (SplitValue.size() >= 4 && SplitValue[3] == "=")
	{
		ValueString = str::Trim(DefinitionSource.substr(DefinitionSource.find_first_of("=") + 1));

		if (Type == "vec3")
		{
			DefinitionSource = str::ReplaceChar(DefinitionSource, ';', ' ');
			if (ValueString.size() < 6)
			{
				return ShaderUniform();
			}
			if (ValueString.substr(0, 5) != "vec3(")
			{
				return ShaderUniform();
			}
			ValueString = ValueString.substr(5);
			ValueString = str::ReplaceChar(ValueString.substr(0, ValueString.size() - 1), ',', ' ');
		}
	}

	return ShaderUniform{
		.Type = TypeNames[Type],
		.Name = Name,
		.DefaultValue = ValueString,
	};
}

void engine::graphics::ShaderModuleLoader::LoadModule(const string& Source, ShaderModule& Info)
{
	bool IsVertex = Info.Type == ShaderModule::ShaderType::Vertex;

	Info.ModuleObject = glCreateShader(IsVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
	const char* SourcePointer = Source.c_str();
	glShaderSource(Info.ModuleObject, 1, &SourcePointer, nullptr);
	glCompileShader(Info.ModuleObject);
	ShaderObject::CheckCompileErrors(Info.ModuleObject, IsVertex ? "VERTEX" : "FRAGMENT");
}

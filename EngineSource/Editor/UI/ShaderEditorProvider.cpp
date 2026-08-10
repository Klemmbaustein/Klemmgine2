#include "ShaderEditorProvider.h"
#include <Engine/Graphics/ShaderLoader.h>
#include <Core/File/FileUtil.h>

engine::editor::ShaderEditorProvider::ShaderEditorProvider(string FilePath, kui::Vec3f VariableColor,
	kui::Vec3f FunctionColor)
	: EngineTextEditorProvider(FilePath)
{
	IsFragment = file::Extension(FilePath) == "frag";

	this->VariableColor = VariableColor;
	this->FunctionColor = FunctionColor;
	Reload();
}

void engine::editor::ShaderEditorProvider::Reload()
{
	Keywords = {
		"attribute", "uniform",
		"varying", "layout",
		"centroid", "flat",
		"smooth", "noperspective",
		"patch", "sample",
		"subroutine", "in",
		"out", "inout",
		"invariant", "discard",
		"mat2", "mat3", "mat4",
		"dmat2", "dmat3", "dmat4",
		"mat2x2", "mat2x3", "mat2x4",
		"dmat2x2", "dmat2x3", "dmat2x4",
		"mat3x2", "mat3x3", "mat3x4",
		"dmat3x2", "dmat3x3", "dmat3x4", "mat4x2",
		"mat4x3", "mat4x4", "dmat4x2", "dmat4x3",
		"dmat4x4", "vec2", "vec3", "vec4", "ivec2", "ivec3",
		"ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3",
		"dvec4", "uvec2", "uvec3", "uvec4", "lowp", "mediump", "highp",
		"precision", "sampler1D", "sampler2D", "sampler3D", "samplerCube",
		"sampler1DArray", "sampler2DArray", "isampler1D",
		"isampler2D", "isampler3D", "isamplerCube", "isampler1DArray", "isampler2DArray",
		"usampler1D", "usampler2D", "usampler3D", "usamplerCube",
		"usampler1DArray", "usampler2DArray", "sampler2DRect", "sampler2DRectShadow",
		"isampler2DRect", "usampler2DRect", "samplerBuffer", "isamplerBuffer",
		"usamplerBuffer", "sampler2DMS", "isampler2DMS", "usampler2DMS",
		"sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray",
		"samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray",
		"usamplerCubeArray",
		"auto",
		"break",
		"case",
		"char",
		"const",
		"continue",
		"default",
		"do",
		"double",
		"else",
		"enum",
		"extern",
		"float",
		"for",
		"goto",
		"if",
		"inline",
		"int",
		"long",
		"register",
		"restrict",
		"return",
		"short",
		"signed",
		"sizeof",
		"static",
		"struct",
		"switch",
		"typedef",
		"union",
		"unsigned",
		"void",
		"volatile",
		"while",
		"#param",
		"#export",
		"#module",
		"#using"
	};

	LoadedShader = graphics::ShaderLoader::Current->Modules.ParseShader(GetContent(),
		IsFragment ? graphics::ShaderModule::ShaderType::Fragment : graphics::ShaderModule::ShaderType::Vertex);

	for (auto& i : LoadedShader.ShaderUniforms)
	{
		Keywords.insert({ i.Name, VariableColor, 1 });
	}

	std::vector<string> BuiltInFunctions = {
		"acos",
		"acosh",
		"asin",
		"asinh",
		"atan",
		"atanh",
		"cos",
		"cosh",
		"degrees",
		"radians",
		"sin",
		"sinh",
		"tan",
		"tanh",
		"abs",
		"ceil",
		"clamp",
		"dFdx",
		"dFdy",
		"exp",
		"exp2",
		"floor",
		"floor",
		"fma",
		"fract",
		"fwidth",
		"inversesqrt",
		"isinf",
		"isnan",
		"log",
		"log2",
		"max",
		"min",
		"mix",
		"mod",
		"modf",
		"noise",
		"pow",
		"round",
		"roundEven",
		"sign",
		"smoothstep",
		"sqrt",
		"step",
		"trunc",
		"floatBitsToInt",
		"frexp",
		"intBitsToFloat",
		"ldexp",
		"packDouble2x32",
		"packHalf2x16",
		"packUnorm",
		"unpackDouble2x32",
		"unpackHalf2x16",
		"unpackUnorm",
		"cross"
		"distance",
		"dot",
		"equal",
		"faceforward",
		"length",
		"normalize",
		"notEqual",
		"reflect",
		"refract",
		"all",
		"any",
		"greaterThan",
		"greaterThanEqual",
		"lessThan",
		"lessThanEqual",
		"not",
		"interpolateAtCentroid",
		"interpolateAtOffset",
		"interpolateAtSample",
		"texelFetch",
		"texelFetchOffset",
		"texture",
		"textureGather",
		"textureGatherOffset",
		"textureGatherOffsets",
		"textureGrad",
		"textureGradOffset",
		"textureLod",
		"textureLodOffset",
		"textureOffset",
		"textureProj",
		"textureProjGrad",
		"textureProjGradOffset",
		"textureProjLod",
		"textureProjLodOffset",
		"textureProjOffset",
		"textureQueryLevels",
		"textureQueryLod",
		"textureSamples",
		"textureSize",
		"determinant",
		"groupMemoryBarrier",
		"inverse",
		"matrixCompMult",
		"outerProduct",
		"transpose",
	};

	for (auto& i : LoadedShader.DependencyModules)
	{
		for (auto& d : i.Exported)
		{
			if (d.Name.empty())
			{
				continue;
			}
			if (d.IsUniform)
			{
				Keywords.insert({ d.Name, VariableColor, 1 });
			}
			else
			{
				Keywords.insert({ d.Name, FunctionColor, 2 });
			}
		}
	}

	for (auto& i : BuiltInFunctions)
	{
		Keywords.insert({ i, FunctionColor, 2 });
	}
}

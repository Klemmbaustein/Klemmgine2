#include "EmbeddedResourceSource.h"
#include <Engine/File/ModelData.h>
#include <kui/Resource.h>

using namespace engine;

bool engine::resource::EmbeddedResourceSource::FileExists(string Path)
{
	if (Path.size() > 4 && Path.substr(0, 4) == "res:")
	{
		return kui::resource::ResourceExists(Path.substr(4));
	}

	return false;
}

IBinaryStream* engine::resource::EmbeddedResourceSource::GetFile(string Path)
{
	if (Path.size() > 4 && Path.substr(0, 4) == "res:")
	{
		if (kui::resource::ResourceExists(Path.substr(4)))
		{
			auto BinaryData = kui::resource::GetBinaryResource(Path.substr(4));

			ReadOnlyBufferStream* Stream = new ReadOnlyBufferStream(BinaryData.Data, BinaryData.FileSize,
				BinaryData.ResourceType == SIZE_MAX);
			return Stream;
		}
	}

	return nullptr;
}

std::map<string, string> engine::resource::EmbeddedResourceSource::GetFiles()
{
	static std::map<string, string> Files = {
		{"res:DefaultFont.ttf", "res:DefaultFont.ttf"},
		{"res:basic.frag", "res:shader/basic.frag"},
		{"res:basic.vert", "res:shader/basic.vert"},
		{"res:sky.frag", "res:shader/sky.frag"},
		{"<Builtin Cube>.kmdl", GraphicsModel::DEFAULT_CUBE_NAME},
		{"<Builtin Plane>.kmdl", GraphicsModel::DEFAULT_PLANE_NAME},
	};

	return Files;
}

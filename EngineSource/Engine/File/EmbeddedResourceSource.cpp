#include "EmbeddedResourceSource.h"
#include <Engine/File/ModelData.h>

using namespace engine;

bool engine::resource::EmbeddedResourceSource::FileExists(string Path)
{
	return false;
}

IBinaryStream* engine::resource::EmbeddedResourceSource::GetFile(string Path)
{
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

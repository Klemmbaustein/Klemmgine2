#include "EmbeddedResourceSource.h"

using namespace engine;

bool engine::resource::EmbeddedResourceSource::FileExists(string Path)
{
	return false;
}

ReadOnlyBufferStream* engine::resource::EmbeddedResourceSource::GetFile(string Path)
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
	};

	return Files;
}

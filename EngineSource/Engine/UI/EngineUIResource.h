#pragma once
#include <kui/UIResourceProvider.h>

namespace engine
{
	class EngineUIResource : public kui::resource::UIResourceProvider
	{
	public:

		// Inherited via UIResourceProvider
		bool FileExists(const std::string& Path) override;
		kui::resource::BinaryData GetFile(const std::string& File) override;
	};
}
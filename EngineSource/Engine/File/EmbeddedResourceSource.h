#include "Resource.h"

namespace engine::resource
{
	class EmbeddedResourceSource : public ResourceSource
	{
	public:


		// Inherited via ResourceSource
		bool FileExists(string Path) override;

		IBinaryStream* GetFile(string Path) override;

		std::map<string, string> GetFiles() override;

	};
}
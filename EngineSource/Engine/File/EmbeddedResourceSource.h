#include "Resource.h"

namespace engine::resource
{
	class EmbeddedResourceSource : public ResourceSource
	{
	public:


		// Inherited via ResourceSource
		bool FileExists(string Path) override;

		ReadOnlyBufferStream* GetFile(string Path) override;

		std::map<string, string> GetFiles() override;

	};
}
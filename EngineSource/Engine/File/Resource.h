#include <Core/Types.h>
#include <map>

namespace engine::resource
{
	[[nodiscard]]
	string GetTextFile(string EnginePath);
	[[nodiscard]]
	string ConvertFilePath(string EnginePath, bool AllowFile = false);

	[[nodiscard]]
	bool FileExists(string EnginePath);

	struct BinaryFile
	{
		const uByte* DataPtr = nullptr;
		size_t DataSize = 0;
		bool CanFree = true;
	};

	[[nodiscard]]
	BinaryFile GetBinaryFile(string EnginePath);
	void FreeBinaryFile(BinaryFile& Target);

	extern std::map<string, string> LoadedAssets;

	void ScanForAssets();
}
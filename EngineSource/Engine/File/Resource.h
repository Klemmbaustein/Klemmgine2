#include <Engine/Types.h>

namespace engine::resource
{
	string GetTextFile(string EnginePath);
	string ConvertFilePath(string EnginePath, bool AllowFile = false);

	struct BinaryFile
	{
		uByte* DataPtr = nullptr;
		size_t DataSize = 0;
		bool CanFree = false;
	};

	BinaryFile GetBinaryFile(string EnginePath);
	void FreeBinaryFile(BinaryFile Target);
}
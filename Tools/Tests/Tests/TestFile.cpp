#include "EngineTest.h"
#include <Core/File/FileUtil.h>
#include <sstream>

int main()
{
	using namespace engine;

	ENGINE_BEGIN_TESTS();

	ENGINE_TEST(FileUtil, "Test FileNameWithoutExt")
	{
		TEST_EXPECT(file::FileNameWithoutExt("Some/path.with.dot/file"), "file");
		TEST_EXPECT(file::FileNameWithoutExt("Some/path/file.ext"), "file");
		TEST_EXPECT(file::FileNameWithoutExt("Some/path/file.ext.2"), "file.ext");

		TEST_EXPECT(file::FilePath("Some/path.with.dot/file.ext"), "Some/path.with.dot");
		TEST_EXPECT(file::FilePath("path"), "path");

		TEST_SUCCESS();
	};

	ENGINE_TEST(FileUtil, "Test FileName")
	{
		TEST_EXPECT(file::FileName("Some/path.with.dot/file"), "file");
		TEST_EXPECT(file::FileName("Some/path/file.ext"), "file.ext");
		TEST_EXPECT(file::FileName("Some/path/file.ext.2"), "file.ext.2");

		TEST_SUCCESS();
	};

	ENGINE_TEST(FileUtil, "Test FilePath")
	{
		TEST_EXPECT(file::FilePath("Some/path.with.dot/file.ext"), "Some/path.with.dot");
		TEST_EXPECT(file::FilePath("path"), "path");

		TEST_SUCCESS();
	};

	ENGINE_END_TESTS();
}
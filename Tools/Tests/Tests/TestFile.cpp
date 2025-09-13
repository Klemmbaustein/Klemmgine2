#include "EngineTest.h"
#include <Core/File/FileUtil.h>
#include <Core/Archive/Archive.h>
#include <sstream>

int main()
{
	using namespace engine;

	ENGINE_BEGIN_TESTS();

	ENGINE_TEST(FileUtil, "Test FileNameWithoutExt")
	{
		TEST_EXPECT_STR(file::FileNameWithoutExt("Some/path.with.dot/file"), "file");
		TEST_EXPECT_STR(file::FileNameWithoutExt("Some/path/file.ext"), "file");
		TEST_EXPECT_STR(file::FileNameWithoutExt("Some/path/file.ext.2"), "file.ext");

		TEST_EXPECT_STR(file::FilePath("Some/path.with.dot/file.ext"), "Some/path.with.dot");
		TEST_EXPECT_STR(file::FilePath("path"), "path");

		TEST_SUCCESS();
	};

	ENGINE_TEST(FileUtil, "Test FileName")
	{
		TEST_EXPECT_STR(file::FileName("Some/path.with.dot/file"), "file");
		TEST_EXPECT_STR(file::FileName("Some/path/file.ext"), "file.ext");
		TEST_EXPECT_STR(file::FileName("Some/path/file.ext.2"), "file.ext.2");

		TEST_SUCCESS();
	};

	ENGINE_TEST(FileUtil, "Test FilePath")
	{
		TEST_EXPECT(file::FilePath("Some/path.with.dot/file.ext"), "Some/path.with.dot");
		TEST_EXPECT(file::FilePath("path"), "path");

		TEST_SUCCESS();
	};

	ENGINE_TEST(Serialize, "Test writing and reading archive binaries")
	{
		string Message = "Test";
		string FileName = "test.txt";

		BufferStream ArchiveBuffer;
		auto TestArchive = new Archive();

		BufferStream FileContent;
		FileContent.WriteString(Message);
		FileContent.ResetStreamPosition();
		TestArchive->AddFile(FileName, &FileContent);

		TestArchive->Save(&ArchiveBuffer);

		delete TestArchive;

		ArchiveBuffer.ResetStreamPosition();

		TestArchive = new Archive(&ArchiveBuffer);

		IBinaryStream* File = TestArchive->GetFile(FileName);

		TEST_EXPECT_STR(File->ReadString(), Message);

		File = nullptr;
		delete TestArchive;

		TEST_SUCCESS();
	};

	ENGINE_END_TESTS();
}
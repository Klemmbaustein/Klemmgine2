#include "EngineTest.h"
#include <Core/Platform/Platform.h>

int main()
{
	using namespace engine::internal::platform;

	ENGINE_BEGIN_TESTS();

	ENGINE_TEST(FileUtil, "Test platform::LoadSharedLibrary")
	{
#if WINDOWS
		auto lib = LoadSharedLibrary("C:/Windows/System32/user32.dll");
		TEST_ASSERT(lib != nullptr);

		auto fn = GetLibraryFunction(lib, "GetDpiForSystem");
		TEST_ASSERT(fn != nullptr);

		// Process isn't DPI-aware, so the return value should always be 96
		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforsystem
		uint32 DpiValue = ((uint32(*)())fn)();
		TEST_ASSERT(DpiValue == 96);

		UnloadSharedLibrary(lib);
#endif

		TEST_SUCCESS();
	};

	ENGINE_END_TESTS();
}
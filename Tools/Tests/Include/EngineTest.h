#pragma once
#include <vector>
#include <functional>
#include <Core/Log.h>
#include <Core/File/SerializedData.h>
#include <Core/Types.h>

namespace engine::test
{
	struct Result
	{
		bool Success = true;
		string ErrorMessage;
	};

	using TestFunction = std::function<Result()>;

	struct Test
	{
		TestFunction fn;
		string Name;
	};

	struct TestManager
	{
		std::vector<Test> Tests;
		const char* NextName = "";

		void operator+=(const TestFunction& func)
		{
			Tests.push_back(Test{
				.fn = func,
				.Name = NextName
				});
		}

		void Run()
		{
			for (Test& i : Tests)
			{
				try
				{
					auto Res = i.fn();
					if (!Res.Success)
					{
						Log::Error(str::Format("! Test '%s' failed: %s", i.Name.c_str(), Res.ErrorMessage.c_str()));
						exit(1);
					}
					Log::Info(str::Format("- Test '%s' passed", i.Name.c_str()));
				}
				catch (std::exception& e)
				{
					Log::Error(str::Format("! Test '%s' failed: %s", i.Name.c_str(), e.what()));
					exit(1);
				}
				catch (SerializeException& e)
				{
					Log::Error(str::Format("! Test '%s' failed: %s", i.Name.c_str(), e.what()));
					exit(1);
				}
			}
		}
	};
}

#define ENGINE_BEGIN_TESTS() engine::test::TestManager _tests
#define ENGINE_TEST(Name, descr) _tests.NextName = descr ; _tests += []() -> engine::test::Result
#define ENGINE_END_TESTS() _tests.Run();

#define TEST_SUCCESS() return engine::test::Result{.Success = true}
#define TEST_FAIL(msg) return engine::test::Result{.Success = false, .ErrorMessage = msg}
#define TEST_EXPECT(value, expect) if (value != expect) TEST_FAIL(engine::str::Format("\n\t%s != %s", # value, # expect))
#define TEST_ASSERT(cond) if (!(cond)) TEST_FAIL(engine::str::Format("\n\tAssert failed: %s", # cond))
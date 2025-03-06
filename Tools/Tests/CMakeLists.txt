option(TEST_CORE "Test KlemmgineCore library" ON)

add_library(EngineTestUtil INTERFACE)
target_include_directories(EngineTestUtil INTERFACE "Include")
target_link_libraries(EngineTestUtil INTERFACE KlemmgineCore)

function(engine_test TEST_NAME TEST_SOURCE)
	add_executable("${TEST_NAME}" ${TEST_SOURCE})
	target_link_libraries("${TEST_NAME}" PRIVATE EngineTestUtil)

	add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")
endfunction()

if(TEST_CORE)
	engine_test("Test.Core.Serialization" "Tests/TestSerialize.cpp")
	engine_test("Test.Core.FileUtil" "Tests/TestFile.cpp")
	engine_test("Test.Core.Platform" "Tests/TestPlatform.cpp")
endif()
#include "EngineTest.h"
#include <Core/File/TextSerializer.h>
#include <Core/File/BinarySerializer.h>
#include <Core/File/JsonSerializer.h>
#include <sstream>

using namespace engine;

auto TestStructure = SerializedValue({
	SerializedData("int", 15),
	SerializedData("float", 10.0f),
	SerializedData("bool", true),
	SerializedData("Vector2", SerializedValue(Vector2(1, 5))),
	});

int main()
{

	ENGINE_BEGIN_TESTS();

	ENGINE_TEST(Serialize, "Serialize and de-serialize text")
	{
		std::stringstream stream;

		string First = TestStructure.ToString(0);

		TextSerializer::ToStream(TestStructure.GetObject(), stream);

		string Second = SerializedValue(TextSerializer::FromStream(stream)).ToString(0);

		if (First != Second)
		{
			TEST_FAIL(str::Format("Serializing and DeSerializing text does not return the same data.\n\t1: %s\n\t2: %s",
				First.c_str(), Second.c_str()));
		}

		TEST_SUCCESS();
	};

	ENGINE_TEST(Serialize, "Serialize and de-serialize binary")
	{
		string First = TestStructure.ToString(0);

		BufferStream BinaryBuffer;
		BinarySerializer::ToBinaryData(TestStructure.GetObject(), &BinaryBuffer);

		BinaryBuffer.ResetStreamPosition();
		string Second = SerializedValue(BinarySerializer::FromStream(&BinaryBuffer)).ToString(0);

		if (First != Second)
		{
			TEST_FAIL(str::Format("Serializing and DeSerializing text does not return the same data.\n\t1: %s\n\t2: %s",
				First.c_str(), Second.c_str()));
		}

		TEST_SUCCESS();
	};

	ENGINE_TEST(Serialize, "Serialize and de-serialize JSON")
	{
		constexpr const char* TEST_JSON = R"({"value": true, "object": {"hello": 1}, "array": [1, 2, 3]})";

		std::stringstream Stream;
		Stream << TEST_JSON;

		Stream.seekg(0);

		auto Result = JsonSerializer::FromStream(Stream);

		TEST_EXPECT(Result.At("value").GetBool(), true);
		TEST_EXPECT(Result.At("object").GetObject().size(), 1);
		TEST_EXPECT(Result.At("array").GetArray().size(), 3);

		TEST_SUCCESS();
	};

	ENGINE_END_TESTS();
}
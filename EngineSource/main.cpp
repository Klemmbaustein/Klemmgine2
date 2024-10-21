#include <iostream>
#include "Engine/Engine.h"
#include <Engine/File/SerializedData.h>
#include <Engine/File/BinarySerializer.h>

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;

	Engine* Instance = Engine::Init();

	SerializedData Data;
	Data.Name = "Testing";
	Data.Value = SerializedValue(
		{
			SerializedData("Test", SerializedValue("Hi")),
			SerializedData("Value 2", SerializedValue("Hello")),
		});

	std::cout << Data.ToString(0);

	BinarySerializer::ToFile({ Data }, "Test.k2b");

	auto Out = BinarySerializer::FromFile("test.k2b", "k2b");
	std::cout << Out.ToString(0);

	Instance->Run();

	return 0;
}
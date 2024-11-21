#include "ObjectReflection.h"
#include <iostream>

uByte engine::Reflection::Register(string Name, std::function<SceneObject* ()> NewFunc, string Category)
{
	std::cout << Name << std::endl;
	return 0;
}

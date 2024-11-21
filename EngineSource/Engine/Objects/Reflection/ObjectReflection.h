#pragma once
#include <Engine/Types.h>
#include <functional>

namespace engine
{
	class SceneObject;
#define REGISTER_OBJECT(name, fullName) inline uByte _ ## name ## _refl_\
 = ::engine::Reflection::Register(# name, []() -> ::engine::SceneObject* { \
	return static_cast<engine::SceneObject*>(new fullName());\
}, # fullName)

	class Reflection
	{
	public:
		static uByte Register(string Name, std::function<SceneObject* ()> NewFunc, string Category = "");

		struct ObjectInfo
		{
			uint32 TypeID;
			string Name;
			string Path;
		};
	};
}
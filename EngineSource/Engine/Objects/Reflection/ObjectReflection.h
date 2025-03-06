#pragma once
#include <Core/Types.h>
#include <functional>
#include <unordered_map>

namespace engine
{
	class SceneObject;
#define ENGINE_OBJECT(name, path) private: static ::engine::SceneObject* Internal_Reflect_CreateNewInst() { \
	return reinterpret_cast<::engine::SceneObject*>(new name());\
}\
public: static inline const volatile ObjectTypeID ObjectType\
	 = ::engine::Reflection::RegisterObjectMacro(# name, Internal_Reflect_CreateNewInst, path)

	using ObjectTypeID = int32;

	class Reflection
	{
	public:

		static ObjectTypeID RegisterObjectMacro(string Name, std::function<SceneObject* ()> NewFunc, string Category = "");
		static ObjectTypeID RegisterObject(string Name, std::function<SceneObject* ()> NewFunc, string Category = "");

		struct ObjectInfo
		{
			ObjectTypeID TypeID = 0;
			string Name;
			string Path;

			std::function<SceneObject* ()> CreateInstance;
		};

		static void Init();

		static std::unordered_map<ObjectTypeID, ObjectInfo> ObjectTypes;

	private:
		static std::vector<ObjectInfo>* MacroTypes;
	};
}
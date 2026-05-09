#pragma once
#include <Core/Types.h>
#include <functional>
#include <unordered_map>
#include <Core/StringUtil.h>
#include <Core/Event.h>

namespace engine
{
	using ObjectTypeID = int32;

	class ReflectionObject
	{
	public:
		virtual ~ReflectionObject() = default;

		ObjectTypeID TypeID = 0;
		Event<> OnDestroyedEvent;
	};

#ifndef ENGINE_PLUGIN
	/**
	* * @def ENGINE_OBJECT(Condition, Description)

	*/
#define ENGINE_OBJECT(name, super, path) private: static ::engine::ReflectionObject* Internal_Reflect_CreateNewInst() { \
	return reinterpret_cast<::engine::ReflectionObject*>(new name());\
}\
public: static inline const volatile ObjectTypeID ObjectType\
	 = ::engine::Reflection::RegisterObjectMacro(# name, Internal_Reflect_CreateNewInst, path, str::Hash(super))

#else
#define ENGINE_OBJECT(name, super, path)
#endif

#define SERIALIZE_PROPERTY(Type, Name, Value) ObjProperty<Type> Name = ObjProperty<Type>(# Name, Value, this)


	class Reflection
	{
	public:

		static ObjectTypeID RegisterObjectMacro(string Name, std::function<ReflectionObject* ()> NewFunc,
			string Category = "", int32 Super = 0);
		static ObjectTypeID RegisterObject(string Name, std::function<ReflectionObject* ()> NewFunc,
			int32 Super, string Category = "");
		static void UnRegisterObject(ObjectTypeID Id);

		struct ObjectInfo
		{
			ObjectTypeID TypeID = 0;
			string Name;
			string Path;
			ObjectTypeID SuperClass = 0;

			bool IsSubclassOf(ObjectTypeID Class) const;

			std::function<ReflectionObject* ()> CreateInstance;
		};

		static void Init();

		static std::unordered_map<ObjectTypeID, ObjectInfo> ObjectTypes;

	private:
		static std::vector<ObjectInfo>* MacroTypes;
	};
}
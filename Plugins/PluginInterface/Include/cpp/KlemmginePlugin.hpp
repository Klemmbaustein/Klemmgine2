#pragma once
#include <functional>
#include <string>
#include <Core/Types.h>
#include <Core/Vector.h>
#include <Engine/Objects/Reflection/ObjectReflection.h>

#if _WIN32
#define ENGINE_EXPORT extern "C" __declspec(dllexport)
#else
#define ENGINE_EXPORT extern "C"
#endif

namespace engine
{
	class SceneObject;
	class Scene;
}

#include "../../Include/internal/InterfaceStruct.hpp"

namespace engine
{
	engine::ObjectTypeID RegisterObject(string Name, std::function<SceneObject*()> CreateInstance, string Category);

	namespace log
	{
		void Info(string Message);
	}

	namespace plugin
	{
		EnginePluginInterface* GetInterface();
	}
}

ENGINE_EXPORT void RegisterTypes();

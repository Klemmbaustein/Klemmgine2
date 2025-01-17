#include "KlemmginePlugin.hpp"

static engine::plugin::EnginePluginInterface Context;

std::vector<std::function<engine::SceneObject* ()>*> CreateObjectFunctions;

ENGINE_EXPORT void PluginLoad(engine::plugin::EnginePluginInterface* Interface)
{
	Context = *Interface;
}

ENGINE_EXPORT void PluginUnload()
{
	for (auto* i : CreateObjectFunctions)
	{
		delete i;
	}
}

engine::ObjectTypeID engine::RegisterObject(std::string Name, std::function<SceneObject* ()> CreateInstance)
{
	auto* FunctionCopy = new std::function(CreateInstance);

	return Context.RegisterObj(Name, [](void* Function)
		{
			auto* fn = reinterpret_cast<std::function<SceneObject*()>*>(Function);
			auto val = (*fn)();
			return val;
		}, FunctionCopy);

	CreateObjectFunctions.push_back(FunctionCopy);
}

void engine::log::Info(std::string Message)
{
	Context.Log(Message.c_str());
}

engine::plugin::EnginePluginInterface* engine::plugin::GetInterface()
{
	return &Context;
}

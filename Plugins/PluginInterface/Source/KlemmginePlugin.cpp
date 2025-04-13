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

engine::ObjectTypeID engine::RegisterObject(string Name, std::function<SceneObject* ()> CreateInstance, string Category)
{
	auto* FunctionCopy = new std::function(CreateInstance);

	return Context.RegisterObj(Name.c_str(), [](void* Function)
		{
			auto* fn = reinterpret_cast<std::function<SceneObject*()>*>(Function);
			auto val = (*fn)();
			return val;
		}, FunctionCopy, Category.c_str());

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

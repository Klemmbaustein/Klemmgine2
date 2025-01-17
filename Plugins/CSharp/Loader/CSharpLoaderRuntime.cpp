#include "CSharpLoaderRuntime.h"
#include "Utils.h"
#include "KlemmginePlugin.hpp"
#include <filesystem>

engine::cSharp::netString RuntimeString(std::string From)
{
#if _WIN32
	return engine::cSharp::StrToWstr(From);
#else
	return From;
#endif
}

std::string FromRuntimeString(engine::cSharp::netString From)
{
#if _WIN32
	return engine::cSharp::WstrToStr(From);
#else
	return From;
#endif
}

engine::cSharp::CSharpLoaderRuntime::CSharpLoaderRuntime(const std::vector<NativeFunction>& Functions)
{
	LoadHostFxr();

	string Path = plugin::GetInterface()->PluginPath;

	LoadFunction = LoadDotNetAssembly(RuntimeString(Path) + NET_STR("/bin/net8.0/") + RuntimeString(ASSEMBLY_NAME) + NET_STR(".runtimeconfig.json"));

	LoadFunctions(Functions);
	void* LoadEngineFunction = LoadCSharpFunction(
		"LoadEngine",
		"Engine.Core.Native",
		"");

	StaticCall<void>(LoadEngineFunction);

	CreateObjectFunction = LoadCSharpFunction(
		"CreateObjectInstance",
		"Engine.Core.ObjectTypes",
		"Engine.Core.ObjectTypes+CreateObjectInstanceDelegate");
}

load_assembly_and_get_function_pointer_fn engine::cSharp::CSharpLoaderRuntime::LoadDotNetAssembly(netString config_path)
{
	// Load .NET Core
	void* LoadAssemblyGetFunctionPointer = nullptr;

	int rc = InitDotNetFunction(config_path.c_str(), nullptr, &HostFxrContext);

	if (rc != 0 || HostFxrContext == nullptr)
	{
		log::Info("Init failed");
		CloseDotNetFunction(HostFxrContext);
		return nullptr;
	}

	// Get the load assembly function pointer
	rc = GetDelegateFunction(
		HostFxrContext,
		hdt_load_assembly_and_get_function_pointer,
		&LoadAssemblyGetFunctionPointer);

	CloseDotNetFunction(HostFxrContext);
	return (load_assembly_and_get_function_pointer_fn)LoadAssemblyGetFunctionPointer;
}

void engine::cSharp::CSharpLoaderRuntime::LoadHostFxr()
{
	char_t buffer[4096];
	size_t buffer_size = sizeof(buffer) / sizeof(char_t);
	//	get_hostfxr_parameters parameters =
	//	{
	//		sizeof(hostfxr_initialize_parameters),
	//		nullptr,
	//		"Path"
	//	};
	int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
	if (rc != 0)
	{
		log::Info("Could not get hostfxr path - Error code: " + std::to_string(rc) + " : " + std::to_string(buffer_size));
		return;
	}
	log::Info("Using .NET runtime: '" + FromRuntimeString(buffer) + "'");

	// Load hostfxr and get desired exports
	HostFxrLibrary = LoadShared(FromRuntimeString(buffer));
	InitDotNetFunction = (hostfxr_initialize_for_runtime_config_fn)GetSharedExport(HostFxrLibrary, "hostfxr_initialize_for_runtime_config");
	GetDelegateFunction = (hostfxr_get_runtime_delegate_fn)GetSharedExport(HostFxrLibrary, "hostfxr_get_runtime_delegate");
	CloseDotNetFunction = (hostfxr_close_fn)GetSharedExport(HostFxrLibrary, "hostfxr_close");
}

void* engine::cSharp::CSharpLoaderRuntime::LoadCSharpFunction(string Function, string Namespace, string Delegate)
{
	void* OutFunction = nullptr;

	//ENGINE_ASSERT(pos != netString::npos, "Root path isn't valid");

	string Path = plugin::GetInterface()->PluginPath;
	
	netString LibraryPath = RuntimeString(Path) + NET_STR("/bin/net8.0/") + RuntimeString(ASSEMBLY_NAME) + RuntimeString(ASSEMBLY_EXT);
	netString NamespacePath = RuntimeString(Namespace) + NET_STR(", ") + RuntimeString(ASSEMBLY_NAME);

	int rc = LoadFunction(
		LibraryPath.c_str(),
		NamespacePath.c_str(),
		RuntimeString(Function).c_str(),
		Delegate.empty() ? UNMANAGEDCALLERSONLY_METHOD : RuntimeString(Delegate + ", " + ASSEMBLY_NAME).c_str(),
		nullptr,
		(void**)&OutFunction);

	if (rc || !OutFunction)
	{
		log::Info(str::Format("Failed to load function: %s.%s (Error code 0x%x)", Namespace.c_str(), Function.c_str(), rc));
		return nullptr;
	}

	return OutFunction;
}

size_t engine::cSharp::CSharpLoaderRuntime::CreateObjectInstance(size_t TypeId)
{
	return size_t();
}

void engine::cSharp::CSharpLoaderRuntime::LoadFunctions(const std::vector<NativeFunction>& Functions)
{
	void* RegisterFunction = LoadCSharpFunction(
		"RegisterFunctions",
		"Engine.Core.Native",
		"Engine.Core.Native+RegisterFunctionDelegate");

	StaticCall<void, NativeFunction*, int>(
		RegisterFunction,
		const_cast<NativeFunction*>(Functions.data()),
		int(Functions.size()));
}

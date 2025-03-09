#include "CSharpLoaderAot.h"
#include <KlemmginePlugin.hpp>
using namespace engine::internal::platform;
using namespace engine;

const string MainFunctionName = "Aot_Main";

engine::cSharp::CSharpLoaderAot::CSharpLoaderAot(const std::vector<NativeFunction>& Functions)
{
	AotLibrary = LoadSharedLibrary(string(plugin::GetInterface()->PluginPath) + "bin/net8.0/publish/win-x64/Klemmgine.CSharp.Aot.dll");

	if (!AotLibrary)
	{
		log::Info("Failed to load Klemmgine.CSharp.Aot.dll");
		return;
	}

	AotMainFn MainFn = AotMainFn(GetLibraryFunction(AotLibrary, MainFunctionName));

	if (!MainFn)
	{
		log::Info(str::Format("Failed to find function %s in the C# assembly.", MainFunctionName.c_str()));
		return;
	}

	MainFn(size_t(Functions.data()), uint32(Functions.size()));

	CreateObject = AotCreateObjectFn(GetLibraryFunction(AotLibrary, "Aot_CreateObjectInstance"));
	DestroyObject = AotDestroyObjectFn(GetLibraryFunction(AotLibrary, "Aot_RemoveObjectInstance"));
	UpdateAot = AotUpdateFn(GetLibraryFunction(AotLibrary, "Aot_Update"));

	if (!UpdateAot || !DestroyObject || !CreateObject)
	{
		log::Info("Failed to load Klemmgine.CSharp.Aot functions");
		return;
	}
}

engine::cSharp::CSharpLoaderAot::~CSharpLoaderAot()
{
	UnloadSharedLibrary(AotLibrary);
}

size_t engine::cSharp::CSharpLoaderAot::CreateObjectInstance(size_t TypeId, void* NativeObject)
{
	return CreateObject(TypeId, NativeObject);
}

void engine::cSharp::CSharpLoaderAot::RemoveObjectInstance(size_t ObjId)
{
	DestroyObject(ObjId);
}

void engine::cSharp::CSharpLoaderAot::Update(float Delta)
{
	if (UpdateAot)
		UpdateAot(Delta);
}

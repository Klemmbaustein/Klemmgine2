#pragma once
#include <Core/Types.h>
#include "CSharpLoader.h"
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
#include "Functions.h"

namespace engine::cSharp
{

#if _WIN32
	using netString = std::wstring;
#define fs_string() wstring()
#define NET_STR(x) L ## x
#else
	using netString = std::string;
#define fs_string() string()
#define NET_STR(x) x
#endif
	class CSharpLoaderRuntime : public CSharpLoader
	{
	public:
		
		static inline const string ASSEMBLY_NAME = "Klemmgine.CSharp.Core";
		static inline const string ASSEMBLY_EXT = ".dll";

		CSharpLoaderRuntime(const std::vector<NativeFunction>& Functions);
		load_assembly_and_get_function_pointer_fn LoadDotNetAssembly(netString config_path);
		void LoadHostFxr();
		void* LoadCSharpFunction(string Function, string Namespace, string Delegate);

		template<typename T, typename... Args>
		static T StaticCall(void* Function, Args... argument)
		{
			if (!Function) abort();
			typedef T(HOSTFXR_CALLTYPE* f)(Args...);
			f fPtr = (f)Function;
			return fPtr(argument...);
		}

		size_t CreateObjectInstance(size_t TypeId, void* NativeObject) override;
		void RemoveObjectInstance(size_t ObjId) override;

		void LoadFunctions(const std::vector<NativeFunction>& Functions);
		void Update(float Delta) override;

	private:
		load_assembly_and_get_function_pointer_fn LoadFunction = nullptr;
		hostfxr_initialize_for_runtime_config_fn InitDotNetFunction = nullptr;
		hostfxr_get_runtime_delegate_fn GetDelegateFunction = nullptr;
		hostfxr_close_fn CloseDotNetFunction = nullptr;
		hostfxr_handle HostFxrContext = nullptr;
		void* HostFxrLibrary = nullptr;

		void* CreateObjectFunction = nullptr;
		void* DestroyObjectFunction = nullptr;
		void* UpdateEngineFunction = nullptr;
	};
}
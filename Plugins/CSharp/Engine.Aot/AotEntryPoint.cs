using Engine.Async;
using Engine.Internal;
using Engine.Native;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Engine.Aot;

internal class AotEntryPoint
{
	[StructLayout(LayoutKind.Sequential)]
	public struct NativeFunctionStruct
	{
		[MarshalAs(UnmanagedType.LPUTF8Str)]
		public string Name;
		public IntPtr FunctionPointer;
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_Main")]
	[RequiresUnreferencedCode("Calls NativeFunctions.RegisterFunctions()")]
	public static void EntryPoint(IntPtr functionPointers, int nativeFunctionCount)
	{
		List<NativeFunctionInfo> engineFunctions = [];

		for (int i = 0; i < nativeFunctionCount; i++)
		{
			IntPtr ptr = functionPointers + Marshal.SizeOf<NativeFunctionStruct>() * i;

			NativeFunctionStruct func = Marshal.PtrToStructure<NativeFunctionStruct>(ptr);
			engineFunctions.Add(new NativeFunctionInfo
			{
				Name = func.Name,
				FunctionPointer = func.FunctionPointer
			});
		}

		NativeFunctions.RegisterFunctions([.. engineFunctions]);
		AotObjects.LoadObjects();
		EngineInternal.Initialize();

		Log.Info("Using AOT published C#");
	}

	[UnmanagedCallersOnly(EntryPoint = "Aot_Update")]
	public static void Update(float delta)
	{
		EngineInternal.Update(delta);
		AotObjects.UpdateObjects();
	}
}

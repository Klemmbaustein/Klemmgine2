using System.Reflection;
using System.Runtime.InteropServices;
namespace Engine.Core;

public class Native
{
	readonly static Dictionary<string, IntPtr> LoadedFunctions = [];

	public delegate void RegisterFunctionDelegate([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] Target, int _);

	public delegate void LogFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Text);

	public static LogFunction? Log;

	private delegate void ObjectFunction();

	[StructLayout(LayoutKind.Sequential)]
	public struct NativeFunction
	{
		[MarshalAs(UnmanagedType.LPUTF8Str)]
		public string Name;
		public IntPtr FunctionPointer;
	}

	public static void RegisterFunctions([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] Target, int _)
	{
		foreach (var Function in Target)
		{
			LoadedFunctions.Add(Function.Name, Function.FunctionPointer);
		}
	}

	public static Delegate? GetFunction<T>(string Name)
	{
		return Marshal.GetDelegateForFunctionPointer<T>(LoadedFunctions[Name]) as Delegate;
	}

	[UnmanagedCallersOnly]
	public static void LoadEngine()
	{
		Log = GetFunction<LogFunction>("Log")! as LogFunction;

		Assembly Engine = Assembly.LoadFrom(Directory.GetCurrentDirectory() + "/Script/bin/net8.0/Klemmgine.CSharp.dll"); ;
		Assembly ProjectAssembly = Assembly.LoadFile(Directory.GetCurrentDirectory() + "/Script/bin/net8.0/CSharpAssembly.dll");

		ObjectTypes.LoadObjects(ProjectAssembly, Engine);
	}
}

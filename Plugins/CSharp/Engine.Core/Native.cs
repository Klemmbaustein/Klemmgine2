using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Engine.Core;

public class Native
{
	public delegate void RegisterFunctionDelegate([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] Target, int _);
	public delegate void LogFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Text);

	public static LogFunction? Log;
	readonly static Dictionary<string, IntPtr> LoadedFunctions = [];

	private static MethodInfo? UpdateFunction = null;

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

	static void NativeFunctionsToEngine(Assembly Target)
	{
		Type? ArrayType = Target.GetType("Engine.Native.NativeFunctionInfo");

		Array NativeArray = Array.CreateInstance(ArrayType!, LoadedFunctions.Count);
		var NameField = ArrayType!.GetField("Name", BindingFlags.Instance | BindingFlags.Public)!;
		var PointerField = ArrayType!.GetField("FunctionPointer", BindingFlags.Instance | BindingFlags.Public)!;

		int i = 0;
		foreach (var fn in LoadedFunctions)
		{
			var NewElement = Activator.CreateInstance(ArrayType);
			NameField.SetValue(NewElement, fn.Key);
			PointerField.SetValue(NewElement, fn.Value);

			NativeArray.SetValue(NewElement, i++);
		}

		Target.GetType("Engine.Native.NativeFunctions")!
			.GetMethod("RegisterFunctions", BindingFlags.Static | BindingFlags.Public)!
			.Invoke(null, [NativeArray, 0]);
	}

	[UnmanagedCallersOnly]
	public static void LoadEngine()
	{
		Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
		Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

		Log = GetFunction<LogFunction>("Log")! as LogFunction;

		Assembly Engine = Assembly.LoadFrom(Directory.GetCurrentDirectory() + "/Script/bin/net8.0/Klemmgine.CSharp.dll"); ;
		Assembly ProjectAssembly = Assembly.LoadFile(Directory.GetCurrentDirectory() + "/Script/bin/net8.0/CSharpAssembly.dll");

		try
		{
			Engine.GetType("Engine.Internal.EngineInternal")!.GetMethod("Initialize")!.Invoke(null, []);
			UpdateFunction = Engine.GetType("Engine.Internal.EngineInternal")!.GetMethod("Update")!;
			ObjectTypes.LoadObjects(ProjectAssembly, Engine);
			NativeFunctionsToEngine(Engine);
		}
		catch (Exception e)
		{
			Log!(e.ToString());
		}
	}

	[UnmanagedCallersOnly]
	public static void UpdateEngine(float delta)
	{
		UpdateFunction!.Invoke(null, [delta]);
		ObjectTypes.UpdateObjects();
	}
}

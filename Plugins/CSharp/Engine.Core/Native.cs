using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Engine.Core;

public class Native
{
	public delegate void RegisterFunctionDelegate([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] target, int _);
	public delegate void LogFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string text);

	public static LogFunction? log;
	readonly static Dictionary<string, IntPtr> loadedFunctions = [];

	private static MethodInfo? updateFunction = null;

	[StructLayout(LayoutKind.Sequential)]
	public struct NativeFunction
	{
		[MarshalAs(UnmanagedType.LPUTF8Str)]
		public string name;
		public IntPtr functionPointer;
	}

	public static void RegisterFunctions([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] target, int _)
	{
		foreach (var function in target)
		{
			loadedFunctions.Add(function.name, function.functionPointer);
		}
	}

	public static Delegate? GetFunction<T>(string name)
	{
		return Marshal.GetDelegateForFunctionPointer<T>(loadedFunctions[name]) as Delegate;
	}

	static void NativeFunctionsToEngine(Assembly Target)
	{
		Type? arrayType = Target.GetType("Engine.Native.NativeFunctionInfo");

		Array nativeArray = Array.CreateInstance(arrayType!, loadedFunctions.Count);
		var nameField = arrayType!.GetField("Name", BindingFlags.Instance | BindingFlags.Public)!;
		var pointerField = arrayType!.GetField("FunctionPointer", BindingFlags.Instance | BindingFlags.Public)!;

		int i = 0;
		foreach (var fn in loadedFunctions)
		{
			var NewElement = Activator.CreateInstance(arrayType);
			nameField.SetValue(NewElement, fn.Key);
			pointerField.SetValue(NewElement, fn.Value);

			nativeArray.SetValue(NewElement, i++);
		}

		Target.GetType("Engine.Native.NativeFunctions")!
			.GetMethod("RegisterFunctions", BindingFlags.Static | BindingFlags.Public)!
			.Invoke(null, [nativeArray, 0]);
	}

	[UnmanagedCallersOnly]
	public static void LoadEngine(IntPtr assemblyPath)
	{
		string assemblyString = Marshal.PtrToStringUTF8(assemblyPath)!;

		Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
		Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

		log = GetFunction<LogFunction>("Log")! as LogFunction;

		try
		{
			Assembly engine = Assembly.LoadFrom(Path.Combine(assemblyString, "Klemmgine.CSharp.dll"));
			Assembly projectAssembly = Assembly.LoadFile(Path.Combine(assemblyString, "GameAssembly.dll"));
			engine.GetType("Engine.Internal.EngineInternal")!.GetMethod("Initialize")!.Invoke(null, []);
			updateFunction = engine.GetType("Engine.Internal.EngineInternal")!.GetMethod("Update")!;
			ObjectTypes.LoadObjects(projectAssembly, engine);
			NativeFunctionsToEngine(engine);
		}
		catch (Exception e)
		{
			log!(e.ToString());
		}
	}

	[UnmanagedCallersOnly]
	public static void UpdateEngine(float delta)
	{
		updateFunction!.Invoke(null, [delta]);
		ObjectTypes.UpdateObjects();
	}
}

using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Engine.Native;

public class NativeFunction
{
	[MarshalAs(UnmanagedType.LPUTF8Str)]
	public string Name = "";
	public IntPtr FunctionPointer;
}

[AttributeUsage(AttributeTargets.Class)]
public class DependsOnNativeAttribute : Attribute
{

}

public class NativeFunctions
{
	readonly static public Dictionary<string, IntPtr> LoadedFunctions = [];
	static List<Type> DependsOnNative = [];

	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(Log))]
	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(SceneObject))]
	[RequiresUnreferencedCode("Uses Assembly.DefinedTypes")]
	public static void RegisterFunctions([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] Target, int _ = 0)
	{
		foreach (var Function in Target)
		{
			LoadedFunctions.Add(Function.Name, Function.FunctionPointer);
		}

		DependsOnNative = [.. Assembly.GetAssembly(typeof(NativeFunctions))!.DefinedTypes
			.Where((type) => type.CustomAttributes
				.Any((attrib) => attrib.Constructor.DeclaringType == typeof(DependsOnNativeAttribute)))];

		foreach (Type NativeDep in DependsOnNative)
		{
			var Func = NativeDep.GetMethod("OnNativeLoaded", BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Static);

			if (Func == null)
			{
				Console.WriteLine($"Type {NativeDep} has DependsOnNative attribute, but doesn't have a static OnNativeLoaded function.");
			}
			else
			{
				Func.Invoke(null, []);
			}
		}
	}

	public static Delegate? GetFunction<T>(string Name)
	{
		if (LoadedFunctions.TryGetValue(Name, out IntPtr Pointer))
		{
			return Marshal.GetDelegateForFunctionPointer<T>(Pointer) as Delegate;
		}
		return null;
	}

}

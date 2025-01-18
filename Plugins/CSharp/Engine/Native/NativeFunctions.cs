using System;
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
	readonly static Dictionary<string, IntPtr> LoadedFunctions = [];
	readonly static List<Type> DependsOnNative = [];
	
	public static void RegisterFunctions([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunction[] Target, int _)
	{
		foreach (var Function in Target)
		{
			LoadedFunctions.Add(Function.Name, Function.FunctionPointer);
		}

		DependsOnNative.Clear();
		foreach (var Type in Assembly.GetAssembly(typeof(NativeFunctions))!.DefinedTypes)
		{
			foreach (var Attrib in Type.CustomAttributes)
			{
				if (Attrib.Constructor.DeclaringType != typeof(DependsOnNativeAttribute))
				{
					continue;
				}
				DependsOnNative.Add(Type);
			}
		}

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
		return Marshal.GetDelegateForFunctionPointer<T>(LoadedFunctions[Name]) as Delegate;
	}

}

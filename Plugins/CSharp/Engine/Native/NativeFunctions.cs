using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.InteropServices;
using Engine.Components;

namespace Engine.Native;

public class NativeFunctionInfo
{
	[MarshalAs(UnmanagedType.LPUTF8Str)]
	public string name = "";
	public IntPtr functionPointer;
}

[AttributeUsage(AttributeTargets.Class)]
public class DependsOnNativeAttribute : Attribute
{

}

public class NativeFunctions
{
	readonly static public Dictionary<string, IntPtr> loadedFunctions = [];
	static List<Type> dependsOnNative = [];

	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(Log))]
	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(SceneObject))]
	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(ObjectComponent))]
	[DynamicDependency(DynamicallyAccessedMemberTypes.NonPublicMethods, typeof(MeshComponent))]
	[RequiresUnreferencedCode("Uses Assembly.DefinedTypes")]
	public static void RegisterFunctions([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] NativeFunctionInfo[] target, int _ = 0)
	{
		foreach (var function in target)
		{
			loadedFunctions.Add(function.name, function.functionPointer);
		}

		dependsOnNative = [.. Assembly.GetAssembly(typeof(NativeFunctions))!.DefinedTypes
			.Where((type) => type.CustomAttributes
				.Any((attrib) => attrib.Constructor.DeclaringType == typeof(DependsOnNativeAttribute)))];

		foreach (Type NativeDep in dependsOnNative)
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

	public static T? GetFunction<T>(string name) where T : Delegate
	{
		if (loadedFunctions.TryGetValue(name, out IntPtr pointer))
		{
			return Marshal.GetDelegateForFunctionPointer<T>(pointer) as T;
		}
		return null;
	}

}

using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Engine.Native;

namespace Engine;

[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public static class Log
{
	delegate void LogFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Text);
	static LogFunction? InfoFunction = null;

	internal static void OnNativeLoaded()
	{
		InfoFunction = NativeFunctions.GetFunction<LogFunction>("Log");
	}

	public static void Info(string Message)
	{
		InfoFunction!(Message);
	}
}

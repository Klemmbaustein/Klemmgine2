using Engine.Native;
using System.Runtime.InteropServices;

namespace Engine;

[DependsOnNative]
public static class Log
{
	delegate void LogFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Text);
	static LogFunction? InfoFunction = null;

#pragma warning disable IDE0051
	private static void OnNativeLoaded()
	{
		InfoFunction = NativeFunctions.GetFunction<LogFunction>("Log")! as LogFunction;
	}
#pragma warning restore IDE0051

	public static void Info(string Message)
	{
		InfoFunction!(Message);
	}
}

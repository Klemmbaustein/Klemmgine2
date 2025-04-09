using Engine.Native;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Engine.Editor;


[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public static class Editor
{
	[return: MarshalAs(UnmanagedType.U1)]
	delegate bool EditorBoolFunction();

	static EditorBoolFunction? IsEditorActiveFunction;

	public static bool IsEditorActive()
	{
		return IsEditorActiveFunction != null && IsEditorActiveFunction();
	}

	internal static void OnNativeLoaded()
	{
		IsEditorActiveFunction = NativeFunctions.GetFunction<EditorBoolFunction>("Log");
	}

}

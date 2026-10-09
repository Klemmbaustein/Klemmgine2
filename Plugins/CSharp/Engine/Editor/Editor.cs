using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Engine.Native;

namespace Engine.Editor;


[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public static class Editor
{
	[return: MarshalAs(UnmanagedType.U1)]
	delegate bool EditorBoolFunction();

	static EditorBoolFunction? isEditorActiveFunction;

	public static bool IsEditorActive()
	{
		return isEditorActiveFunction != null && isEditorActiveFunction();
	}

	internal static void OnNativeLoaded()
	{
		isEditorActiveFunction = NativeFunctions.GetFunction<EditorBoolFunction>("Log");
	}

}

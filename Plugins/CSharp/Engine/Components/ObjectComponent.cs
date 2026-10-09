using Engine.Native;

namespace Engine.Components;

[DependsOnNative]
public class ObjectComponent
{
	public IntPtr nativePointer = 0;

	delegate void AttachComponentDelegate(IntPtr Parent, IntPtr Comp);

	static AttachComponentDelegate? AttachComponent = null;

	internal static void OnNativeLoaded()
	{
		AttachComponent = NativeFunctions.GetFunction<AttachComponentDelegate>("ComponentAttach")!;
	}

	public void Attach(ObjectComponent NewChild)
	{
		AttachComponent!(nativePointer, NewChild.nativePointer);
	}
}

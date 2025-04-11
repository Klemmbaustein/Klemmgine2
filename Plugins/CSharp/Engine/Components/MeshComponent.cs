using Engine.Native;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Engine.Components;

[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public class MeshComponent : ObjectComponent
{
	delegate IntPtr NewMeshComponent();
	delegate void MeshLoadFunction(IntPtr Comp, [MarshalAs(UnmanagedType.LPUTF8Str)] string Str);

	static NewMeshComponent? NewMesh = null;
	static MeshLoadFunction? MeshLoad = null;

	public MeshComponent()
	{
		NativePointer = NewMesh!();
	}

	public void Load(string MeshFile)
	{
		MeshLoad!(NativePointer, MeshFile);
	}

	internal static new void OnNativeLoaded()
	{
		NewMesh = NativeFunctions.GetFunction<NewMeshComponent>("NewMeshComponent");
		MeshLoad = NativeFunctions.GetFunction<MeshLoadFunction>("MeshComponentLoad");
	}
}

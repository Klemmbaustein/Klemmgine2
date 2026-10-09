using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Engine.Native;

namespace Engine.Components;

[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public class MeshComponent : ObjectComponent
{
	delegate IntPtr NewMeshComponent();
	delegate void MeshLoadFunction(IntPtr Comp, [MarshalAs(UnmanagedType.LPUTF8Str)] string Str);

	static NewMeshComponent? newMesh = null;
	static MeshLoadFunction? meshLoad = null;

	public MeshComponent()
	{
		nativePointer = newMesh!();
	}

	public void Load(string MeshFile)
	{
		meshLoad!(nativePointer, MeshFile);
	}

	internal static new void OnNativeLoaded()
	{
		newMesh = NativeFunctions.GetFunction<NewMeshComponent>("NewMeshComponent");
		meshLoad = NativeFunctions.GetFunction<MeshLoadFunction>("MeshComponentLoad");
	}
}

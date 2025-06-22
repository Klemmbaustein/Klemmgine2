using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Engine.Components;
using Engine.Native;

namespace Engine;

public class InvalidObjectException : Exception
{
	public override string Message => "Object does not exists or has been destroyed.";
}

[DependsOnNative]
[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.All)]
public abstract class SceneObject
{
	delegate IntPtr GetNameDelegate(IntPtr NativePointer);
	delegate void ObjectAttachComponent(IntPtr Obj, IntPtr Comp);
	delegate void SetVecDelegate(IntPtr Obj, Vector3 Value);
	delegate Vector3 GetVecDelegate(IntPtr Obj);

	static GetNameDelegate? GetNativeName = null;

	static SetVecDelegate? SetPosition = null;
	static SetVecDelegate? SetRotation = null;
	static SetVecDelegate? SetScale = null;

	static GetVecDelegate? GetPosition = null;
	static GetVecDelegate? GetRotation = null;
	static GetVecDelegate? GetScale = null;

	static ObjectAttachComponent? AttachComponent = null;

	public IntPtr CSharpType;
	public IntPtr NativePointer;

	private readonly CancellationTokenSource objCancellationSource = new();

	public CancellationToken ObjCancellationToken
	{
		get
		{
			return objCancellationSource.Token;
		}
	}

	public Vector3 Position
	{
		get
		{
			return GetPosition!(NativePointer);
		}

		set
		{
			SetPosition!(NativePointer, value);
		}
	}
	public Vector3 Rotation
	{
		get
		{
			return GetRotation!(NativePointer);
		}

		set
		{
			SetRotation!(NativePointer, value);
		}
	}


	public string Name
	{
		get
		{
			ThrowIfInvalid();
			return Marshal.PtrToStringUTF8(GetNativeName!(NativePointer)) ?? "Unknown";
		}
	}

	[EditorBrowsable(EditorBrowsableState.Never)]
	public void BeginInternal()
	{
		Begin();
		BeginAsync();
	}

	public virtual void Begin()
	{
	}

	public virtual Task BeginAsync()
	{
		return Task.CompletedTask;
	}

	public abstract void Update();

	[EditorBrowsable(EditorBrowsableState.Never)]
	public void OnDestroyedInternal()
	{
		OnDestroyed();
		OnDestroyedAsync();
		objCancellationSource.Cancel();
	}

	public virtual void OnDestroyed()
	{
	}

	public virtual Task OnDestroyedAsync()
	{
		return Task.CompletedTask;
	}

	public void ThrowIfInvalid()
	{
		if (NativePointer == 0)
			throw new InvalidObjectException();
	}

	public void Attach(ObjectComponent NewComponent)
	{
		AttachComponent!(NativePointer, NewComponent.NativePointer);
	}

	internal static void OnNativeLoaded()
	{
		GetNativeName = NativeFunctions.GetFunction<GetNameDelegate>("GetObjName");
		AttachComponent = NativeFunctions.GetFunction<ObjectAttachComponent>("ObjectAttachComponent");

		SetPosition = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectPosition");
		SetRotation = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectRotation");
		SetScale = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectScale");

		GetPosition = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectPosition");
		GetRotation = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectRotation");
		GetScale = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectScale");

	}
}

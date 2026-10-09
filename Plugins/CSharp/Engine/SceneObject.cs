using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
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

	static GetNameDelegate? getNativeName = null;

	static SetVecDelegate? setPosition = null;
	static SetVecDelegate? setRotation = null;
	static SetVecDelegate? setScale = null;

	static GetVecDelegate? getPosition = null;
	static GetVecDelegate? getRotation = null;
	static GetVecDelegate? getScale = null;

	static ObjectAttachComponent? AttachComponent = null;

	public IntPtr CSharpType;
	public IntPtr nativePointer;

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
			return getPosition!(nativePointer);
		}

		set
		{
			setPosition!(nativePointer, value);
		}
	}
	public Vector3 Rotation
	{
		get
		{
			return getRotation!(nativePointer);
		}

		set
		{
			setRotation!(nativePointer, value);
		}
	}

	public Vector3 Scale
	{
		get
		{
			return getScale!(nativePointer);
		}

		set
		{
			setScale!(nativePointer, value);
		}
	}

	public string Name
	{
		get
		{
			ThrowIfInvalid();
			return Marshal.PtrToStringUTF8(getNativeName!(nativePointer)) ?? "Unknown";
		}
	}

	[EditorBrowsable(EditorBrowsableState.Never)]
	public void BeginInternal()
	{
		Begin();
		BeginAsync().Start();
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
		OnDestroyedAsync().Start();
		objCancellationSource.Cancel();
	}

	public virtual void OnDestroyed()
	{
	}

	public virtual Task OnDestroyedAsync()
	{
		return Task.CompletedTask;
	}

	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public void ThrowIfInvalid()
	{
		if (nativePointer == 0)
			throw new InvalidObjectException();
	}

	public void Attach(ObjectComponent NewComponent)
	{
		AttachComponent!(nativePointer, NewComponent.nativePointer);
	}

	internal static void OnNativeLoaded()
	{
		getNativeName = NativeFunctions.GetFunction<GetNameDelegate>("GetObjName");
		AttachComponent = NativeFunctions.GetFunction<ObjectAttachComponent>("ObjectAttachComponent");

		setPosition = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectPosition");
		setRotation = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectRotation");
		setScale = NativeFunctions.GetFunction<SetVecDelegate>("SetObjectScale");

		getPosition = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectPosition");
		getRotation = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectRotation");
		getScale = NativeFunctions.GetFunction<GetVecDelegate>("GetObjectScale");

	}
}

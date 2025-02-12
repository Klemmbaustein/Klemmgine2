using Engine.Native;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

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
	static GetNameDelegate? GetNativeName = null;

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

	public string Name
	{
		get
		{
			ThrowIfInvalid();
			return Marshal.PtrToStringUTF8(GetNativeName!(NativePointer)) ?? "Unknown";
		}
	}

	internal static void OnNativeLoaded()
	{
		GetNativeName = NativeFunctions.GetFunction<GetNameDelegate>("GetObjName")! as GetNameDelegate;
	}
}

using System.Runtime.InteropServices;

namespace Engine;

public abstract class SceneObject
{
	IntPtr CSharpType;

	public abstract void Begin();
	public abstract void Update();
	public abstract void OnDestroyed();
}

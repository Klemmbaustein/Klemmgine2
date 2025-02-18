using Engine;
using Engine.Components;

namespace Game;

public class CoolObject : SceneObject
{

	public override async Task BeginAsync()
	{
		MeshComponent c = new();
		Attach(c);
		c.Load("cube.kmdl");

		await Task.Yield();
	}

	public override void Update()
	{
	}

	public override void OnDestroyed()
	{
	}
}

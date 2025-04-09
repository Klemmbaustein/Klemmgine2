using Engine;
using Engine.Components;

namespace Game;

public class CoolObject : SceneObject
{
	float time = 0;

	public override void Begin()
	{
		MeshComponent c = new();
		Attach(c);
		c.Load("cube.kmdl");
		Log.Info("Object created");
	}

	public override void Update()
	{
		Rotation = new Vector3(0, time, 0);
		time += Stats.DeltaTime * 90;
	}

	public override void OnDestroyed()
	{
	}
}

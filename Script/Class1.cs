using Engine;

namespace Game;

public class CoolObject : SceneObject
{

	public override void Begin()
	{
		Log.Info("Hai!");
	}

	public override void Update()
	{
	}

	public override void OnDestroyed()
	{
		Log.Info("Bye!");
	}
}

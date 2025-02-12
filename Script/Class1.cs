using Engine;

namespace Game;

public class CoolObject : SceneObject
{
	public override async Task BeginAsync()
	{
		Log.Info("1");

		await Task.Delay(500);

		Log.Info("2");
	}

	public override void Update()
	{
	}

	public override void OnDestroyed()
	{
		Log.Info("Bye!");
	}
}

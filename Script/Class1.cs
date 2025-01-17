using Engine;

namespace Project;

public class CoolObject : SceneObject
{
	public void Begin()
	{
		Console.WriteLine("hi!");
	}

	public void Update()
	{
	}

	public void OnDestroyed()
	{
		Console.WriteLine("bye!");
	}
}

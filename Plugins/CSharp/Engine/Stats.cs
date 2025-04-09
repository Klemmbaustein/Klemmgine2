using Engine.Internal;

namespace Engine;

public static class Stats
{
	public static float DeltaTime
	{
		get
		{
			return EngineInternal.frameDelta;
		}
	}
}

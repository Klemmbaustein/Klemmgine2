namespace Engine;

public static class Stats
{
	public static float DeltaTime
	{
		get
		{
			return Internal.EngineInternal.frameDelta;
		}
	}
}

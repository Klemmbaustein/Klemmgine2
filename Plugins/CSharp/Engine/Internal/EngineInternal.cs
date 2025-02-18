using Engine.Async;

namespace Engine.Internal;

public static class EngineInternal
{
	internal static float frameDelta = 0;
	public static void Initialize()
	{
		SynchronizationContext.SetSynchronizationContext(new EngineSyncContext());
	}

	public static void Update(float delta)
	{
		frameDelta = delta;
		EngineAsyncTasks.Update();
	}
}

using Engine.Async;

namespace Engine.Internal;

public static class EngineInternal
{
	public static void Initialize()
	{
		SynchronizationContext.SetSynchronizationContext(new EngineSyncContext());
	}

	public static void Update(float _)
	{
		EngineAsyncTasks.Update();
	}
}

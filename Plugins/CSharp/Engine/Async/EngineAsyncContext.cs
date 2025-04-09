namespace Engine.Async;

public class EngineSyncContext() : SynchronizationContext
{
	public override void Send(SendOrPostCallback d, object? state)
	{
		EngineAsyncTasks.Queue(d, state, true);
	}

	public override void Post(SendOrPostCallback d, object? state)
	{
		EngineAsyncTasks.Queue(d, state, false);
	}
}
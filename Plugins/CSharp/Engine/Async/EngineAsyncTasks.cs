using System.Collections.Concurrent;

namespace Engine.Async;

public static class EngineAsyncTasks
{
	static readonly ConcurrentQueue<KeyValuePair<SendOrPostCallback, object?>> queuedTasks = [];

	public static void Queue(SendOrPostCallback callBack, object? state, bool instant)
	{
		if (EngineAsync.IsMainThread && instant)
		{
			callBack(state);
		}
		else
		{
			queuedTasks.Enqueue(new KeyValuePair<SendOrPostCallback, object?>(callBack, state));
		}
	}

	public static void Update()
	{
		while (queuedTasks.TryDequeue(out var callback))
		{
			callback.Key(callback.Value);
		}
	}
}

namespace Engine.Async;

public static class EngineAsync
{
	[ThreadStatic]
	public static bool IsMainThread;

	static EngineAsync()
	{
		IsMainThread = false;
	}

	public class NotOnMainThreadException : Exception
	{
		public override string Message => "Code expected to run on the main thread, but didn't!";
	}

	public static void ThrowIfNotOnMainThread()
	{
		if (!IsMainThread)
			throw new NotOnMainThreadException();
	}
}

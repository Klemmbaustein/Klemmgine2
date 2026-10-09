namespace Engine.Async;

public static class EngineAsync
{
	[ThreadStatic]
	public static bool isMainThread;

	static EngineAsync()
	{
		isMainThread = false;
	}

	public class NotOnMainThreadException : Exception
	{
		public override string Message => "Code expected to run on the main thread, but didn't!";
	}

	public static void ThrowIfNotOnMainThread()
	{
		if (!isMainThread)
			throw new NotOnMainThreadException();
	}
}

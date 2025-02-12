#include "EngineError.h"
#include <csignal>
#include <Core/Error/StackTrace.h>
#include <map>
#include <Core/Log.h>
using namespace engine;

static thread_local engine::string ThreadName;
static thread_local bool Crashed = false;
static thread_local bool SignalReceived = false;

static void OnErrorCallbackInternal(string Error, string StackTrace)
{
	std::vector<Log::LogPrefix> Prefixes = {
		Log::LogPrefix{
			.Text = "Critical",
			.Color = Log::LogColor::Magenta
	},
		Log::LogPrefix{
			.Text = "Crash",
			.Color = Log::LogColor::Magenta
	},
	};

	Log::PrintMsg(Error, Log::LogColor::Red, Prefixes);
	Log::PrintMsg("Stack trace:\n" + StackTrace, Log::LogColor::Red, Prefixes);
}

std::function<void(string Error, string StackTrace)> error::OnErrorCallback = &OnErrorCallbackInternal;

static void SignalHandler(int Signal)
{
	static std::map<int, const char*> SignalTypes =
	{
		{ SIGABRT, "SIGABRT" },
		{ SIGFPE, "Math error" },
		{ SIGILL, "Illegal instruction" },
		{ SIGINT, "SIGINT" },
		{ SIGSEGV, "Access violation" },
		{ SIGTERM, "SIGTERM" },
	};

	if (SignalReceived)
	{
		return;
	}
	SignalReceived = true;

	engine::error::Abort(engine::str::Format("Signal raised: %s", SignalTypes[Signal]));
}

void engine::error::InitForThread(string ThreadName)
{
	Crashed = false;
	::ThreadName = ThreadName;
	signal(SIGSEGV, &SignalHandler);
	signal(SIGABRT, &SignalHandler);
}

engine::string engine::error::GetThreadName()
{
	return ::ThreadName;
}

void engine::error::Abort(string Message)
{
	using namespace engine;

	if (Crashed)
	{
		return;
	}
	Crashed = true;

	OnErrorCallback(Message, GetStackTrace());
	abort();
}

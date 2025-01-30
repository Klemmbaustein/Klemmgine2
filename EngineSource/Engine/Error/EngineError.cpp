#include "EngineError.h"
#include <csignal>
#include <Engine/Error/StackTrace.h>
#include <map>
#include <Engine/Engine.h>
#include <Engine/Log.h>
#include <Engine/MainThread.h>

#if !defined(ENGINE_UTILS_LIB) && !defined(SERVER)
#include <kui/App.h>
#include <Engine/Subsystem/VideoSubsystem.h>
#endif

static thread_local engine::string ThreadName;
static thread_local bool Crashed = false;
static thread_local bool SignalReceived = false;

#define ERRORS_TO_MESSAGEBOX 1

static void SignalHandler(int Signal)
{
	static std::map<int, const char*> SignalTypes =
	{
		{SIGABRT, "SIGABRT"},
		{SIGFPE, "Math error"},
		{SIGILL, "Illegal instruction"},
		{SIGINT, "SIGINT"},
		{SIGSEGV, "Access violation"},
		{SIGTERM, "SIGTERM"},
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

	std::vector<Log::LogPrefix> Prefixes = {
		Log::LogPrefix{
			.Text = "Critical",
			.Color = Log::LogColor::Red
		},
		Log::LogPrefix{
			.Text = "Crash",
			.Color = Log::LogColor::Red
		},
	};

#if !defined(ENGINE_UTILS_LIB) && !defined(SERVER)
	if (thread::IsMainThread)
	{
		subsystem::VideoSubsystem* VideoSys = Engine::GetSubsystem<subsystem::VideoSubsystem>();

		if (VideoSys)
		{
			delete VideoSys->MainWindow;
		}
	}
#endif

	Log::PrintMsg(Message, Log::LogColor::Red, Prefixes);
	string Trace = error::GetStackTrace();
	Log::PrintMsg("Stack trace:\n" + Trace, Log::LogColor::Red, Prefixes);

#if !defined(ENGINE_UTILS_LIB) && !defined(SERVER)
	kui::app::MessageBox(str::Format("%s!\nStack trace:\n%s", Message.c_str(), Trace.c_str()), "Engine error", kui::app::MessageBoxType::Error);
	abort();
#endif
}

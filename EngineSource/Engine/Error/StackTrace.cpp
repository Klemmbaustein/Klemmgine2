#include "StackTrace.h"
#include <version>
#include <sstream>
#include <vector>
#include <Engine/Log.h>
#include <Engine/File/FileUtil.h>
#if __cpp_lib_stacktrace >= 202011L && !LINUX
#define HAS_CPP_STACKTRACE 1
#endif

#if HAS_CPP_STACKTRACE
#include <stacktrace>
#elif LINUX
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <cxxabi.h>
#endif


engine::string engine::error::GetStackTrace()
{
	std::stringstream OutStream;
	struct StackFunction
	{
		string Name;
		string Address;
	};

	std::vector<StackFunction> Stack;
	bool HasDebugInfo = false;

#if HAS_CPP_STACKTRACE

	auto Current = std::stacktrace::current();


	for (auto& i : Current)
	{
		string FileName = file::FileName(i.source_file());

		if (FileName == "StackTrace.cpp" || FileName == "EngineError.cpp")
			continue;

		if (!FileName.empty())
		{
			string FunctionName = i.description();
#if WINDOWS
			FunctionName = FunctionName.substr(FunctionName.find_first_of("!") + 1);
			FunctionName = FunctionName.substr(0, FunctionName.find_first_of("+"));
			if (i.source_file().substr(i.source_file().find_last_of(".") + 1) == "inl")
				continue;
#endif
			
			FileName = str::Format("%s(): %s, line %i", FunctionName.c_str(), FileName.c_str(), int(i.source_line()));
			HasDebugInfo = true;
		}

		Stack.push_back(StackFunction{
			.Name = FileName,
			.Address = i.description(),
			});
	}
#elif LINUX

	void* trace[64];
	char** messages;

	int TraceSize = backtrace(trace, 64);
	messages = backtrace_symbols(trace, TraceSize);
	for (int i = 2; i < TraceSize; i++)
	{
		// Format: /path/to/binary(functionName()+0x1234) []
		string Entry = messages[i];

		try
		{
			string Name = Entry.substr(Entry.find_last_of("(") + 1);
			Name = Name.substr(0, Name.find_first_of(")"));
			size_t PlusLocation = Name.find_last_of("+");
			string SymbolName = Name.substr(0, PlusLocation);

			string EntryName = SymbolName;

			if (!EntryName.empty())
			{
				int st;
				char* DeMangledName = abi::__cxa_demangle
				(
					SymbolName.c_str(),
					nullptr,
					0,
					&st
				);

				if (DeMangledName)
				{
					EntryName = DeMangledName;
					free(DeMangledName);
				}
				else
				{
					EntryName = EntryName + "()";
				}
			}
			Stack.push_back(StackFunction{
				.Name = EntryName,
				.Address = Entry,
				});
		}
		catch (std::exception)
		{
			Stack.push_back(StackFunction{
				.Address = Entry,
				});
		}
	}
	free(messages);
#endif

	for (auto& i : Stack)
	{
		if (HasDebugInfo && i.Name.empty())
			continue;
		OutStream << ("    at: " + (i.Name.empty() ? i.Address : i.Name)) << std::endl;
	}
	return OutStream.str();
}

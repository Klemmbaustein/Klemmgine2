#if WINDOWS
#include "Pipe.h"
#include <Windows.h>
#include <thread>
#include <Core/Log.h>
#include <Core/Platform/Platform.h>

using namespace engine::platform;

Pipe::Pipe(string Command)
{
	SECURITY_ATTRIBUTES secattr;
	ZeroMemory(&secattr, sizeof(secattr));
	secattr.nLength = sizeof(secattr);
	secattr.bInheritHandle = TRUE;

	HANDLE OutRPipe, OutWPipe;
	HANDLE InRPipe, InWPipe;
	HANDLE ErrRPipe, ErrWPipe;

	// Create pipes to write and read data

	if (!CreatePipe(&OutRPipe, &OutWPipe, &secattr, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}
	if (!SetHandleInformation(OutRPipe, HANDLE_FLAG_INHERIT, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}
	if (!CreatePipe(&InRPipe, &InWPipe, &secattr, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}
	if (!SetHandleInformation(InWPipe, HANDLE_FLAG_INHERIT, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}
	if (!CreatePipe(&ErrRPipe, &ErrWPipe, &secattr, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}
	if (!SetHandleInformation(ErrRPipe, HANDLE_FLAG_INHERIT, 0))
	{
		this->ReturnValue = 1;
		this->ErrorMessage = platform::GetLastErrorString();
		this->IsEmpty = true;
		return;
	}

	STARTUPINFOA sInfo;
	ZeroMemory(&sInfo, sizeof(sInfo));
	PROCESS_INFORMATION pInfo;
	ZeroMemory(&pInfo, sizeof(pInfo));
	sInfo.cb = sizeof(sInfo);
	sInfo.dwFlags = STARTF_USESTDHANDLES;
	sInfo.hStdOutput = OutWPipe;
	sInfo.hStdError = ErrWPipe;
	sInfo.hStdInput = InRPipe;

	if (!CreateProcessA(0, Command.data(), 0, 0, TRUE, NORMAL_PRIORITY_CLASS, 0, 0, &sInfo, &pInfo))
	{
		this->ErrorMessage = platform::GetLastErrorString();
		this->ReturnValue = 1;
		this->IsEmpty = true;
	}

	ThreadHandle = pInfo.hThread;
	ProcessHandle = pInfo.hProcess;

	PipeStdOutReadHandle = OutRPipe;
	PipeStdErrReadHandle = ErrRPipe;
	PipeStdInWriteHandle = InWPipe;
	CloseHandle(OutWPipe);
	CloseHandle(ErrWPipe);
	CloseHandle(InRPipe);

	PipeThread = new std::thread(ReadThread, this);
	LogThread = new std::thread(WriteLogThread, this);
}

engine::platform::Pipe::~Pipe()
{
	CloseProcess();
}

void Pipe::Write(std::string Message)
{
	DWORD Written = 0;
	::WriteFile(PipeStdInWriteHandle, Message.data(), DWORD(Message.size()), &Written, FALSE);
}

void Pipe::Update()
{
	if (!IsEmpty)
	{
		IsEmpty = WaitForSingleObject(HANDLE(ProcessHandle), 0) == WAIT_OBJECT_0;
	}

	if (IsEmpty)
	{
		this->CloseProcess();
	}
}

void Pipe::ReadThread(Pipe* Target)
{
	BOOL res;
	char buf[8000];
	DWORD NumRead = 0;
	do
	{
		res = ::ReadFile(Target->PipeStdOutReadHandle, buf, sizeof(buf) - 1, &NumRead, 0);
		if (!res)
		{
			break;
		}

		if (NumRead)
		{
			std::lock_guard MessagesGuard = std::lock_guard(Target->MessagesMutex);
			buf[NumRead] = '\0';
			Target->Received.append(buf);
		}

	} while (NumRead);
	Target->SetEmpty();
}

void Pipe::WriteLogThread(Pipe* Target)
{
	BOOL res;
	char buf[8000];
	DWORD NumRead = 0;
	do
	{
		res = ::ReadFile(Target->PipeStdErrReadHandle, buf, sizeof(buf) - 1, &NumRead, 0);

		if (!res)
		{
			break;
		}
		
		if (NumRead)
		{
			std::lock_guard MessagesGuard = std::lock_guard(Target->MessagesMutex);
			buf[NumRead] = '\0';
			Target->Received.append(buf);
		}

	} while (NumRead);
	Target->SetEmpty();
}

bool engine::platform::Pipe::Empty()
{
	this->Update();
	return this->IsEmpty;
}

void engine::platform::Pipe::SetEmpty()
{
	this->IsEmpty = true;
}

int32 engine::platform::Pipe::GetReturnValue()
{
	return this->ReturnValue;
}

engine::string engine::platform::Pipe::GetErrorMessage()
{
	return this->ErrorMessage;
}

std::vector<engine::string> Pipe::GetNewLines()
{
	std::lock_guard MessagesGuard = std::lock_guard(MessagesMutex);
	
	std::vector<string> Lines;

	string CurrentLine;

	for (char c : Received)
	{
		if (c == '\n')
		{
			Lines.push_back(CurrentLine);
			CurrentLine.clear();
		}
		else
		{
			CurrentLine.push_back(c);
		}
	}

	this->Received = CurrentLine;

	return Lines;
}
void engine::platform::Pipe::CloseProcess()
{
	if (!ProcessHandle)
	{
		return;
	}

	GetExitCodeProcess(ProcessHandle, (LPDWORD)&this->ReturnValue);

	PipeThread->detach();
	LogThread->detach();
	CloseHandle(HANDLE(ProcessHandle));
	CloseHandle(HANDLE(ThreadHandle));

	delete PipeThread;
	delete LogThread;
	CloseHandle(PipeStdOutReadHandle);
	CloseHandle(PipeStdErrReadHandle);
	CloseHandle(PipeStdInWriteHandle);
	ProcessHandle = nullptr;
}
#endif
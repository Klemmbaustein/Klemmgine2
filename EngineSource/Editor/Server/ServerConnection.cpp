#include "ServerConnection.h"
#include <Core/Error/EngineAssert.h>
#include <Core/File/JsonSerializer.h>
#include <Core/Log.h>
#include <Core/Platform/Platform.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/Version.h>
#include <Engine/MainThread.h>
#include <Engine/File/Resource.h>
#include <sstream>
#include <fstream>

engine::editor::ServerConnection::ServerConnection(string Url)
{
	this->Connection = new http::WebSocketConnection("ws://" + Url + "/ws");

	this->Connection->OnOpened = [this] {
		SendMessage("tryConnect", {});
	};

	this->Connection->OnMessage = [this] (const http::WebSocketMessage& msg) {
		MessageType Type = msg.Data->Get<MessageType>();

		switch (Type)
		{
		case engine::editor::ServerConnection::MessageType::JsonMessage:
		{
			try
			{
				std::stringstream String;
				String << msg.Data->ReadString();
				auto Data = JsonSerializer::FromStream(String);
				HandleMessage(Data);
			}
			catch (SerializeReadException& e)
			{
				Log::Warn(e.what());
			}
			catch (SerializeException& e)
			{
				Log::Warn(e.what());
			}
			break;
		}
		case engine::editor::ServerConnection::MessageType::FileUpload:
		{
			string Path = msg.Data->ReadString();

			std::vector<uByte>* Data = new std::vector<uByte>();
			size_t* References = new size_t();

			Data->resize(msg.Data->GetBuffer().size() - msg.Data->StreamPosition);

			if (Data->size())
			{
				msg.Data->Read(Data->data(), Data->size());
			}

			std::lock_guard g{ FilesMutex };

			auto& Callbacks = FileCallbacks[Path];

			for (auto& i : Callbacks)
			{
				auto b = new ReadOnlyBufferStream(Data->data(), Data->size(), true);
				b->OnClose.Add(References, [=] {
					(*References)--;
					if (*References == 0)
					{
						delete Data;
						delete References;
					}
				});
				i(b);
				(*References)++;
			}

			Callbacks.clear();

			break;
		}
		default:
			break;
		}
	};

	this->Connection->OnClosed = [&] {
		IsClosed = true;
	};
}

engine::editor::ServerConnection::~ServerConnection()
{
	OnClosed.Invoke();
	delete this->Connection;
}

void engine::editor::ServerConnection::SendMessageData(SerializedValue Json)
{
	if (IsClosed)
	{
		return;
	}

	BufferStream OutStream;
	// The type of the message, telling it this is a JSON encoded request.
	OutStream.WriteValue(MessageType::JsonMessage);
	// Write JSON terminated by null
	std::stringstream Result;
	JsonSerializer::ToStream(Json, Result);
	OutStream.WriteString(Result.str());
	Connection->Send(OutStream.GetBuffer().data(), OutStream.GetSize(), true);
}

void engine::editor::ServerConnection::SendMessage(string Type, SerializedValue Json)
{
	SendMessageData(SerializedValue({
		SerializedData({"type", Type}),
		SerializedData({"data", Json}),
		}));
}

void engine::editor::ServerConnection::SendFile(string Path, IBinaryStream* Stream, size_t Length)
{
	BufferStream OutStream;
	OutStream.WriteValue(MessageType::FileUpload);
	OutStream.WriteString(Path);

	uByte Buffer[2048];
	size_t ToWrite = Length;
	while (ToWrite > 0)
	{
		size_t ChunkSize = std::min(sizeof(Buffer), ToWrite);
		Stream->Read(Buffer, ChunkSize);
		OutStream.Write(Buffer, ChunkSize);
		ToWrite -= ChunkSize;
	}

	Connection->Send(OutStream.GetBuffer().data(), OutStream.GetSize(), true);
}

void engine::editor::ServerConnection::HandleMessage(SerializedValue Json)
{
	string Type = Json.At("type").GetString();

	if (Type == "connectParams")
	{
		HandleConnectParams(Json);
	}
	else if (Type == "initialized")
	{
		HandleInitializedParams(Json);
	}
	else if (Type == "listFiles")
	{
		HandleFileList(Json.At("data").GetArray());
	}
	else if (Type == "chatMessage")
	{
		auto& MessageData = Json.At("data");

		this->Chat.push_back(ChatMessage{
			.Sender = MessageData.At("sender").GetString(),
			.Message = MessageData.At("content").GetString(),
			});
		thread::ExecuteOnMainThread(std::bind(&Event<>::Invoke, &OnChatMessage));
	}
	else if (Type == "listUsers")
	{
		auto& MessageData = Json.At("data").At("users");

		this->Users.clear();

		for (auto& i : MessageData.GetArray())
		{
			this->Users.push_back(i.GetString());
		}
		thread::ExecuteOnMainThread(std::bind(&Event<>::Invoke, &OnUsersChanged));
	}
	else if (Type == "connectDeny")
	{
		Log::Warn("Connection from server was denied.");
		if (OnConnectionAcceptDeny)
			OnConnectionAcceptDeny(false);
	}
	else if (Type == "connectAllow")
	{
		Log::Info("Connection from server was accepted.");
		if (OnConnectionAcceptDeny)
			OnConnectionAcceptDeny(true);
	}
	else
	{
		Log::Error("Unknown message received from server: " + Type);

		std::stringstream Stream;
		JsonSerializer::ToStream(Json, Stream);

		Log::Error(Stream.str());
	}
}

void engine::editor::ServerConnection::HandleConnectParams(SerializedValue Json)
{
	srand(time(NULL));
	this->ThisUserName = "User " + std::to_string(std::rand() % 256);
	SendMessage("connect", SerializedValue({
		SerializedData("password", "hello"),
		SerializedData("version", VersionInfo::Get().VersionName),
		SerializedData("userName", this->ThisUserName),
		}));
}

void engine::editor::ServerConnection::HandleInitializedParams(SerializedValue Json)
{
	HandleFileList(Json.At("data").At("fileSystem").GetArray());

	auto& UserJson = Json.At("data").At("users").GetArray();

	this->Users.clear();

	for (auto& i : UserJson)
	{
		this->Users.push_back(i.At("name").GetString());
	}

	thread::ExecuteOnMainThread(std::bind(&Event<>::Invoke , &OnUsersChanged));
}

void engine::editor::ServerConnection::GetFile(string Name, std::function<void(ReadOnlyBufferStream*)> Callback)
{
	std::lock_guard g{ FilesMutex };

	FileCallbacks[Name].push_back(Callback);
	SendMessage("getFile", Name);
}

void engine::editor::ServerConnection::HandleFileList(std::vector<SerializedValue> Values)
{
	this->Files.clear();

	for (auto& i : Values)
	{
		this->Files.push_back(i.GetString());
	}

	thread::ExecuteOnMainThread([] {
		resource::ScanForAssets();
		EditorUI::Instance->AssetsProvider->OnChanged.Invoke();
	});
}

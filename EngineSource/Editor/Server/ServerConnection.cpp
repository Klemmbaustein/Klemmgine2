#include "ServerConnection.h"
#include <Core/Error/EngineAssert.h>
#include <Core/File/JsonSerializer.h>
#include <Core/Log.h>
#include <Core/Platform/Platform.h>
#include <Engine/Version.h>
#include <Engine/File/Resource.h>
#include <sstream>
#include <fstream>

engine::editor::ServerConnection::ServerConnection(string Url)
{
	this->Connection = new http::WebSocketConnection("wss://" + Url + "/ws");

	this->Connection->OnOpened = [&] {
		SendMessage("tryConnect", {});
	};

	this->Connection->OnMessage = [&] (const http::WebSocketMessage& msg) {
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
			Data->resize(msg.Data->GetBuffer().size() - msg.Data->StreamPosition);

			if (Data->size())
			{
				msg.Data->Read(Data->data(), Data->size());
			}

			std::lock_guard g{ FilesMutex };

			auto& Callbacks = FileCallbacks[Path];

			auto b =  new ReadOnlyBufferStream(Data->data(), Data->size(), true);

			for (auto& i : Callbacks)
			{
				i(b);
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
	SendMessage("connect", SerializedValue({
		SerializedData("password", "hello"),
		SerializedData("version", VersionInfo::Get().VersionName),
		SerializedData("userName", platform::GetSystemUserName()),
		}));
}

void engine::editor::ServerConnection::HandleInitializedParams(SerializedValue Json)
{
	auto& files = Json.At("data").At("fileSystem").GetArray();
	this->Files.clear();

	for (auto& i : files)
	{
		this->Files.push_back(i.GetString());
	}
	resource::ScanForAssets();
}

void engine::editor::ServerConnection::GetFile(string Name, std::function<void(ReadOnlyBufferStream*)> Callback)
{
	std::lock_guard g{ FilesMutex };

	FileCallbacks[Name].push_back(Callback);
	SendMessage("getFile", Name);
}

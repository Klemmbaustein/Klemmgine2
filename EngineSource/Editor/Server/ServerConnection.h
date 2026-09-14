#pragma once
#include <Core/Networking/HttpWebSocket.h>
#include <Core/File/SerializedData.h>
#include <Core/Event.h>
#include <Core/ThreadMessages.h>
#include <map>
#include <mutex>

namespace engine::editor
{
	struct ChatMessage
	{
		string Sender;
		string Message;
	};

	class ServerConnection
	{
	public:
		ServerConnection(string Url, std::optional<string> Password);
		~ServerConnection();

		http::WebSocketConnection* Connection = nullptr;

		enum class MessageType : uByte
		{
			JsonMessage,
			FileUpload,
		};

		std::function<void(bool)> OnConnectionAcceptDeny;
		bool IsClosed = false;

		std::vector<string> Files;
		std::vector<string> Users;
		std::vector<ChatMessage> Chat;

		Event<> OnUsersChanged;
		Event<> OnChatMessage;
		Event<> OnClosed;

		string ConnectionName;
		string ThisUserName;

		void SendMessageData(SerializedValue Json);
		void SendMessage(string Type, SerializedValue Json);
		void SendFile(string Path, IBinaryStream* Stream, size_t Length);

		void HandleMessage(SerializedValue Json);
		void HandleConnectParams(SerializedValue Json);
		void HandleInitializedParams(SerializedValue Json);

		void GetFile(string Name, std::function<void(ReadOnlyBufferStream*)> Callback);

		void ListenToLiveFile(string FileName, std::function<void(string, SerializedValue&)> Callback,
			thread::ThreadMessagesRef OnContext);

		std::mutex FilesMutex;

		std::map<string, std::vector<std::function<void(ReadOnlyBufferStream*)>>> FileCallbacks;

	private:
		struct CallbackData
		{
			std::function<void(string, SerializedValue&)> Callback;
			thread::ThreadMessagesRef OnContext;
		};

		std::optional<string> Password;
		std::map<std::string, CallbackData> ListenedFiles;
		void HandleFileChanged(string File);
		void HandleFileList(std::vector<SerializedValue> Values);
	};
}
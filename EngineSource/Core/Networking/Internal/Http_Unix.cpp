#if defined(__linux__)
#include <Core/Networking/Http.h>
#include "HttpInternal.hpp"
#include <Core/Closeable.h>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <string.h>
#include <cstddef>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace engine;

using std::size_t;

http::HttpResponse* http::SendRequestNoSSL(HttpUrl Url, string Method, HttpOptions Options)
{
	string Message = Options.GetHeadersString(Url, Method);
	Message.append(Options.Body);

	Closeable SocketFile = { socket(AF_INET, SOCK_STREAM, 0), close };
	if (SocketFile < 0)
	{
		SocketFile.Invalidate();
		return HttpResponse::HttpError("Error opening socket");
	}

	hostent* Server = gethostbyname(Url.HostName.c_str());
	if (!Server)
		return HttpResponse::HttpError("No such host");

	sockaddr_in ServerAddress{ 0 };
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(Url.Port);
	memcpy(&ServerAddress.sin_addr.s_addr, Server->h_addr, Server->h_length);

	/* connect the socket */
	if (connect(SocketFile, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
		return HttpResponse::HttpError("Error connecting");

	/* send the request */
	size_t Sent = 0, Total = Message.size();
	do
	{
		size_t WrittenBytes = write(SocketFile, Message.data() + Sent, Total - Sent);
		if (WrittenBytes < 0)
			return HttpResponse::HttpError("Error writing message to socket");
		if (WrittenBytes == 0)
			break;
		Sent += WrittenBytes;
	} while (Sent < Total);

	return internal::HandleConnection([&SocketFile](int64 ToRead, void* ToPtr) -> int64 {
		return read(SocketFile, ToPtr, ToRead);
	});
}
#endif

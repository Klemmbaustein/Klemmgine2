#if defined(UNIX)
#include "aio/http/http.hpp"
#include "http_internal.hpp"
#include <aio/closeable_value.hpp>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <string.h>
#include <cstddef>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace aio;

using std::size_t;

http::HttpResponsePtr http::sendRequestNoSSL(HttpUrl url, str method, HttpOptions options)
{
	hostent* server;
	sockaddr_in serv_addr;

	/* What are we going to send? */
	str message = options.getHeadersString(url, method);
	message.append(options.body);

	/* create the socket */
	CloseableValue sockfd = { socket(AF_INET, SOCK_STREAM, 0), close };
	if (sockfd < 0)
	{
		sockfd.invalidate();
		return HttpResponse::httpError("Error opening socket");
	}
	/* lookup the ip address */
	server = gethostbyname(url.hostName.c_str());
	if (!server)
		return HttpResponse::httpError("No such host");

	/* fill in the structure */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(url.port);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	/* connect the socket */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		return HttpResponse::httpError("Error connecting");

	/* send the request */
	size_t sent = 0, total = message.size();
	do
	{
		size_t bytes = write(sockfd, message.data() + sent, total - sent);
		if (bytes < 0)
			return HttpResponse::httpError("Error writing message to socket");
		if (bytes == 0)
			break;
		sent += bytes;
	} while (sent < total);

	return internal::handleConnection([&sockfd](i64 toRead, void* to) -> i64 {
		return read(sockfd, to, toRead);
	});
}
#endif

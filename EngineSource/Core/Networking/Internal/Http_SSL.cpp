#if defined(ENGINE_WITH_OPENSSL)
#include <aio/http/http.h>
#include <Core/Log.h>
#include "http_internal.hpp"
#include <Core/Closeable.h>
#include <openssl/ssl.h>
#include "openssl/err.h"

using namespace engine;

using std::size_t;

http::HttpResponsePtr http::SendRequestSSL(HttpUrl url, string method, HttpOptions options)
{
	SSL_library_init();

	Closeable sslContext = { SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free };

	if (!sslContext)
	{
		sslContext.invalidate();
		return HttpResponse::HttpError("Failed to create OpenSSL Context");
	}

	CloseableValue bio = { BIO_new_ssl_connect(sslContext), BIO_free_all };

	BIO_set_conn_hostname(bio, (url.HostName + ":" + std::to_string(url.Port)).c_str());

	int64 result = BIO_do_connect(bio);
	if (result <= 0)
	{
		ERR_print_errors_fp(stderr);
		bio.invalidate();
		return HttpResponse::HttpError(std::format("Failed connection: {}", ERR_error_string(ERR_get_error(), nullptr)));
	}

	string httpMessage = options.getHeadersString(url, method);
	httpMessage.append(options.body);

	if (BIO_write(bio, httpMessage.data(), httpMessage.size()) <= 0)
	{
		return HttpResponse::HttpError("Failed write");
	}

	return internal::handleConnection([&bio](int64 toRead, void* to) -> int64
		{
			return BIO_read(bio, to, toRead);
		});
}
#endif
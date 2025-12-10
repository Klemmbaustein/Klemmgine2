#if defined(AIO_WITH_OPENSSL)
#include <aio/http/http.hpp>
#include <aio/log.hpp>
#include "http_internal.hpp"
#include <aio/closeable_value.hpp>
#include <openssl/ssl.h>
#include "openssl/err.h"

using namespace aio;

using std::size_t;

http::HttpResponsePtr http::SendRequestSSL(HttpUrl url, str method, HttpOptions options)
{
	SSL_library_init();

	CloseableValue sslContext = { SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free };

	if (!sslContext)
	{
		sslContext.invalidate();
		return HttpResponse::httpError("Failed to create OpenSSL Context");
	}

	CloseableValue bio = { BIO_new_ssl_connect(sslContext), BIO_free_all };

	BIO_set_conn_hostname(bio, (url.hostName + ":" + std::to_string(url.port)).c_str());

	i64 result = BIO_do_connect(bio);
	if (result <= 0)
	{
		ERR_print_errors_fp(stderr);
		bio.invalidate();
		return HttpResponse::httpError(std::format("Failed connection: {}", ERR_error_string(ERR_get_error(), nullptr)));
	}

	str httpMessage = options.getHeadersString(url, method);
	httpMessage.append(options.body);

	if (BIO_write(bio, httpMessage.data(), httpMessage.size()) <= 0)
	{
		return HttpResponse::httpError("Failed write");
	}

	return internal::handleConnection([&bio](i64 toRead, void* to) -> i64
		{
			return BIO_read(bio, to, toRead);
		});
}
#endif
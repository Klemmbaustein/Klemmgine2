#if defined(ENGINE_WITH_OPENSSL)
#include <Core/Networking/Http.h>
#include <Core/Log.h>
#include <Core/Networking/Internal/HttpInternal.h>
#include <Core/Closeable.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>
#include "openssl/err.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/x509v3.h>

using namespace engine;

using std::size_t;

http::HttpResponse* http::SendRequestSSL(HttpUrl Url, string Method, HttpOptions Options)
{
	SSL_library_init();

	Closeable SSLContext = { SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free };

	SSL_CTX_set_default_verify_paths(SSLContext);

	if (!SSLContext)
	{
		SSLContext.Invalidate();
		throw HttpException("Failed to create OpenSSL Context");
	}

	Closeable bio = { BIO_new_ssl_connect(SSLContext), BIO_free_all };

	SSL* SSL;
	BIO_get_ssl(bio, &SSL);

	SSL_set_tlsext_host_name(SSL, Url.HostName.c_str());

	BIO_set_conn_hostname(bio, (Url.HostName + ":" + std::to_string(Url.Port)).c_str());

	if (BIO_do_handshake(bio) <= 0)
	{
		throw HttpException("Handshake failed");
	}

	int err = SSL_get_verify_result(SSL);
	if (err != X509_V_OK)
	{
		const char* message = X509_verify_cert_error_string(err);
		throw HttpException(str::Format("Certificate error: %s (%d)", message, err));
	}
	X509* cert = SSL_get_peer_certificate(SSL);
	if (cert == nullptr)
	{
		throw HttpException("Missing certificate");
		exit(1);
	}
	if (X509_check_host(cert, Url.HostName.data(), Url.HostName.size(), 0, nullptr) != 1)
	{
		throw HttpException("Hostname mismatch");
	}

	int64 result = BIO_do_connect(bio);
	if (result <= 0)
	{
		ERR_print_errors_fp(stderr);
		bio.Invalidate();
		throw HttpException(str::Format("Failed connection: %s", ERR_error_string(ERR_get_error(), nullptr)));
	}

	string httpMessage = Options.GetHeadersString(Url, Method);
	httpMessage.append(Options.Body);

	if (BIO_write(bio, httpMessage.data(), httpMessage.size()) <= 0)
	{
		return HttpResponse::HttpError("Failed write");
	}

	return internal::HandleConnection([&bio](int64 toRead, void* to) -> int64 {
		return BIO_read(bio, to, toRead);
	});
}
#endif
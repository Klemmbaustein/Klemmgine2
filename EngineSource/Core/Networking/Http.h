#pragma once
#include <Core/Types.h>
#include <Core/File/BinaryStream.h>
#include <map>
#include <exception>

namespace engine::http
{
	class HttpException : public std::exception
	{
	public:

		HttpException(const string& Message);

		const char* what() const noexcept override;

		string Message = "";
	};

	namespace methods
	{
		constexpr auto GET = "GET";
		constexpr auto POST = "POST";
	}

	enum class HttpStatus
	{
		Continue = 100,
		SwitchingProtocols = 101,

		OK = 200,
		Created = 201,
		Accepted = 202,
		NonAuthInformation = 203,
		NoContent = 204,
		Resetcontent = 205,
		PartialContent = 206,

		MultipleChoice = 300,
		MovedPermanently = 301,
		Found = 302,
		SeeOther = 303,
		NotModified = 304,
		UseProxy = 305,
		SwitchProxy = 306,
		TemporaryRedirect = 307,
		PermanentRedirect = 308,

		BadRequest = 400,
		Unauthorized = 401,
		PaymentRequired = 402,
		Forbidden = 403,
		NotFound = 404,
		MethodNotAllowed = 405,
		NotAcceptable = 406,
		ProxyAuthRequired = 407,
		RequestTimeout = 408,
		Conflict = 409,
		Gone = 410,
		LengthRequired = 411,
		PreconditionFailed = 412,
		PayloadTooLarge = 413,
		UriTooLong = 414,
		UnspupportedMediaType = 415,
		RangeNotSatisfiable = 416,
		ExpectationFailed = 417,
		Teapot = 418,
		MisdirectedRequest = 421,
		UnprocessableContent = 422,
		UpgradeREquired = 426,

		InternalServerError = 500,
		NotImplemented = 501,
		BadGateway = 502,
		ServiceUnavailable = 503,
		GatewayTimeout = 504,
		HttpVersionNotSupported = 505,

		Unknown = 1000,
		InternalError = 1001,
	};

	struct HttpUrl
	{
		HttpUrl(string FromString);
		HttpUrl();

		string HostName;
		string Path;
		string Scheme;
		uint16 Port = 0;

		static constexpr auto SCHEME_HTTPS = "https://";
		static constexpr auto SCHEME_HTTP = "http://";
	};

	struct HttpOptions
	{
		std::map<string, string> Headers;
		string Body;

		string GetHeadersString(HttpUrl Target, string Method) const;
	};

	struct HttpResponse
	{
		/**
		 * @brief
		 * Constructs a HTTP response from the binary data of an HTTP response.
		 *
		 * @param headerData
		 * The first part of the HTTP response before the first CRLF CRLF sequence.
		 *
		 * @param content
		 * The rest of the HTTP response's data.
		 */
		HttpResponse(const std::vector<uByte>& HeaderData, const std::vector<uByte>& Content);

		/**
		 * @brief
		 * Constructs an empty HTTP response.
		 */
		HttpResponse();

		/**
		 * @brief
		 * Constructs a HTTP response from a status code.
		 *
		 * This is meant to be used with the server API to return errors.
		 * The body will contain a generic description of the HTTP status.
		 *
		 * @param status
		 * The status of the response.
		 *
		 * @see HttpServer
		 */
		HttpResponse(HttpStatus Status);

		/**
		 * @brief
		 * Constructs a HTTP response from a status code.
		 *
		 * This is meant to be used with the server API to return raw string content.
		 * The status code will be set to 200 OK.
		 *
		 * @param Body
		 * The body of the response, as an UTF8 string.
		 */
		HttpResponse(string Body);

		/**
		 * @brief
		 * Constructs a HTTP response from a status code.
		 *
		 * This is meant to be used with the server API to return raw string content.
		 *
		 * @param Body
		 * The body of the response, as an UTF8 string.
		 *
		 * @param Status
		 * The status of the response.
		 */
		HttpResponse(string Body, HttpStatus Status);

		/**
		 * @brief
		 * Constructs a HTTP response from a status code.
		 *
		 * This is meant to be used with the server API to return raw string content.
		 *
		 * @param Body
		 * The body of the response.
		 *
		 * @param Status
		 * The status of the response.
		 */
		HttpResponse(IBinaryStream* Body, HttpStatus Status);

		HttpResponse(const HttpResponse&) = default;

		~HttpResponse();

		static HttpResponse* HttpError(string description);

		struct ParseHeaderData
		{
			std::map<string, string> Headers;
			HttpStatus Status;
		};

		static ParseHeaderData ParseHeaders(const std::vector<uByte>& HeaderData);

		static std::pair<string, string> ParseHeader(string FullHeader);

		std::map<string, string> Headers;

		/**
		 * @brief
		 * The body of the HTTP response.
		 */
		IBinaryStream* Body = nullptr;
		HttpStatus Status = HttpStatus::Unknown;
	};

	HttpResponse* SendRequest(string Url, string Method);
	/**
	 * @brief
	 * Sends a HTTP request to a specific URL and returns the response in HttpResponse
	 *
	 * @param Url
	 * The URL the request should be sent to
	 */
	HttpResponse* SendRequest(string Url, string Method, HttpOptions Options);

#ifdef ENGINE_WITH_OPENSSL
	HttpResponse* SendRequestSSL(HttpUrl Url, string Method, HttpOptions Options);
#endif
	HttpResponse* SendRequestNoSSL(HttpUrl Url, string Method, HttpOptions Options);
}
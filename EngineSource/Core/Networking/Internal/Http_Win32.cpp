#if defined(WINDOWS)
#include <Core/Networking/Http.h>
#include <Core/Closeable.h>
#include <Core/Log.h>
#include <cstdlib>
#include <format>
#include <vector>
#include <cstddef>

#include "Http_Win32.h"

#pragma comment(lib, "Wininet.lib")

using namespace engine;
using std::size_t;

// Even though it's called "NoSSL", the windows implementation still supports TLS for HTTPS
http::HttpResponse* http::SendRequestNoSSL(HttpUrl url, string method, HttpOptions options)
{
	auto Connection = win32::HttpConnection(url, method, options);

	if (!Connection.Connection)
	{
		return HttpResponse::HttpError("Failed to open HTTP connection.");
	}

	// Read all the data from the request
	std::vector<uByte> response;
	size_t responsePosition = 0;
	const size_t CHUNK_SIZE = BUFSIZ;

	while (true)
	{
		response.resize(responsePosition + CHUNK_SIZE + 1);
		size_t NumRead = Connection.Connection->ReadCount(&response[responsePosition], CHUNK_SIZE);

		if (NumRead == 0)
		{
			break;
		}
		responsePosition += NumRead;
	}
	response.resize(responsePosition);

	return new HttpResponse(HttpResponse(Connection.HeaderBytes, response));
}
#endif

engine::http::win32::HttpConnection::HttpConnection(HttpUrl url, string method, HttpOptions options)
{
	// Create internet connection
	this->InternetHandle = InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (!this->InternetHandle)
	{
		return;
		//return HttpResponse::HttpError("Failed to open internet handle");
	}

	this->ConnectionHandle = InternetConnectA(this->InternetHandle, url.HostName.c_str(), url.Port, "", "", INTERNET_SERVICE_HTTP, 0, 0);

	if (!this->ConnectionHandle)
	{
		Log::Error("No no connection :(");
		return;
		//connection.Invalidate();
		//return HttpResponse::HttpError("Failed to open connection");
	}

	// Create a request and add headers
	RequestHandle = HttpOpenRequestA(this->ConnectionHandle, method.c_str(), url.Path.c_str(), nullptr, nullptr, nullptr,
		url.Scheme == HttpUrl::SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0, 0);

	if (!RequestHandle)
	{
		Log::Error("No request :(");
		return;
		//request.Invalidate();
		//return HttpResponse::HttpError("Failed to create request");
	}

	for (auto& [name, value] : options.Headers)
	{
		auto newHeader = str::Format("%s: %s", name.c_str(), value.c_str());
		HttpAddRequestHeadersA(RequestHandle, newHeader.c_str(), newHeader.size(), HTTP_ADDREQ_FLAG_ADD);
	}

	// Send the request
	BOOL success = HttpSendRequestA(RequestHandle, NULL, 0, (LPVOID)options.Body.c_str(), options.Body.size());
	if (!success)
	{
		Log::Error("No success :(");
		return;
		//return HttpResponse::HttpError("Failed to send http request");
	}

	this->Connection = new HttpConnectionStream(this);

	// Also read the headers from the request
	DWORD headerSize = 1024;
	HeaderBytes.resize(headerSize);
	while (!HttpQueryInfoA(RequestHandle, HTTP_QUERY_RAW_HEADERS_CRLF, HeaderBytes.data(), &headerSize, 0))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			HeaderBytes.resize(headerSize);
		}
		else
		{
			break;
		}
	}
	HeaderBytes.resize(headerSize);
}

engine::http::win32::HttpConnection::~HttpConnection()
{
	InternetCloseHandle(this->InternetHandle);
	InternetCloseHandle(this->ConnectionHandle);
}
engine::http::win32::HttpConnectionStream::HttpConnectionStream(HttpConnection* Connection)
{
	this->Connection = Connection;
}

engine::http::win32::HttpConnectionStream::~HttpConnectionStream()
{
}

bool engine::http::win32::HttpConnectionStream::IsReadOnly() const
{
	return false;
}

bool engine::http::win32::HttpConnectionStream::IsWriteOnly() const
{
	return false;
}

bool engine::http::win32::HttpConnectionStream::IsEmpty() const
{
	return false;
}

size_t engine::http::win32::HttpConnectionStream::GetSize() const
{
	return size_t();
}

bool engine::http::win32::HttpConnectionStream::Read(uByte* To, size_t Size)
{
	return ReadCount(To, Size) == Size;
}

void engine::http::win32::HttpConnectionStream::Write(uByte* Buffer, size_t Size)
{
	WriteCount(Buffer, Size);
}

size_t engine::http::win32::HttpConnectionStream::WriteCount(uByte* Buffer, size_t Size) const
{
	size_t ResponsePosition = 0;
	while (true)
	{
		DWORD NumWritten = 0;

		bool HasWritten = InternetWriteFile(Connection->RequestHandle, Buffer + ResponsePosition, Size - ResponsePosition, &NumWritten);

		if (!HasWritten)
		{
			Log::Error(std::format("InternetWriteFile error: {}", GetLastError()));
			return ResponsePosition;
		}

		if (NumWritten == 0)
		{
			return ResponsePosition;
		}

		ResponsePosition += NumWritten;

		if (ResponsePosition == Size)
		{
			return ResponsePosition;
		}
	}
}

size_t engine::http::win32::HttpConnectionStream::ReadCount(uByte* To, size_t Size) const
{
	size_t ResponsePosition = 0;
	while (true)
	{
		DWORD numBytesRead;

		bool hasRead = InternetReadFile(Connection->RequestHandle, To + ResponsePosition, Size - ResponsePosition, &numBytesRead);

		if (!hasRead)
		{
			Log::Error(std::format("InternetReadFile error: {}", GetLastError()));
			return ResponsePosition;
		}

		if (numBytesRead == 0)
			return ResponsePosition;
		ResponsePosition += numBytesRead;

		if (ResponsePosition == numBytesRead)
		{
			return ResponsePosition;
		}
	}
}

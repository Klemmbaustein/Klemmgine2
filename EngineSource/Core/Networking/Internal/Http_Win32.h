#if WINDOWS
#pragma once
#include <Core/Networking/Http.h>
#include <Windows.h>
#include <wininet.h>

namespace engine::http::win32
{
	class HttpConnection;
	class HttpConnectionStream : public IBinaryStream
	{
	public:

		HttpConnectionStream(HttpConnection* Connection);
		~HttpConnectionStream();

		virtual bool IsReadOnly() const override;
		virtual bool IsWriteOnly() const override;
		virtual bool IsEmpty() const override;
		virtual size_t GetSize() const override;

		virtual bool Read(uByte* To, size_t Size) override;
		virtual void Write(uByte* Buffer, size_t Size) override;

		size_t WriteCount(uByte* Buffer, size_t Size) const;
		size_t ReadCount(uByte* To, size_t Size) const;

		HttpConnection* Connection = nullptr;
	};

	class HttpConnection
	{
	public:
		HttpConnection(HttpUrl url, string method, HttpOptions options);
		~HttpConnection();

		std::vector<uByte> HeaderBytes;

		HttpConnectionStream* Connection = nullptr;

		HINTERNET InternetHandle = nullptr;
		HINTERNET ConnectionHandle = nullptr;
		HINTERNET RequestHandle = nullptr;
	};
}
#endif
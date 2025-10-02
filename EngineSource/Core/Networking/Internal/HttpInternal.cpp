#include "HttpInternal.hpp"
#include <cstddef>
#include <format>
#include <array>
#include <cstring>
#include <fstream>

using namespace engine::http;
using std::size_t;

// HTTP parsing function (supporting HTTP 1.1 chunked content) used by both the unix and openssl http implementations.
// WinInet already does automatically, so it's not needed in the windows implementation.

HttpResponse* engine::http::internal::HandleConnection(std::function<int64(int64 toRead, void* to)> readFunction)
{
	const size_t CHUNK_SIZE = 1024;

	std::vector<uByte> headers;
	std::vector<uByte> body;
	size_t responsePos = 0;
	uByte last = 0;
	string header = "";
	bool headersEnded = false;

	// SIZE_MAX for chunked
	size_t contentSize = 0;

	// Read headers
	while (!headersEnded)
	{
		headers.resize(headers.size() + CHUNK_SIZE);
		int64 numRead = readFunction(CHUNK_SIZE, &headers[responsePos]);
		// Error reading from the socket
		if (numRead < 0)
			return HttpResponse::HttpError(std::format("Failed to read a response from the socket: {}", strerror(errno)));
		// Done reading from the socket
		if (numRead == 0)
			return HttpResponse::HttpError(std::format("Unexpected EOF before HTTP headers ended"));

		size_t newDataSize = responsePos + numRead;
		for (; responsePos < newDataSize; responsePos++)
		{
			uByte c = headers[responsePos];

			if (c == '\n' && last == '\r')
			{
				if (header.empty())
				{
					responsePos++;
					headersEnded = true;

					size_t restSize = newDataSize - responsePos;
					if (restSize)
					{
						// copy the rest of this chunk into the body (if there's anything left)
						body = { headers.begin() + responsePos, headers.begin() + newDataSize };
					}
					break;
				}

				std::pair newHeader = HttpResponse::ParseHeader(header);
				if (newHeader.first == "content-length")
				{
					contentSize = std::atoi(newHeader.second.c_str());
				}
				else if (newHeader.first == "transfer-encoding" && newHeader.second == "chunked")
				{
					contentSize = SIZE_MAX;
				}

				header.clear();
				last = 0;
				continue;
			}

			last = c;

			if (c != '\r')
			{
				header.push_back(c);
			}
		}
	}
	headers.resize(responsePos);

	// HTTP body has a fixed size declared by content-length: [some_size]
	if (contentSize != SIZE_MAX)
	{
		// Reading the headers also reads parts of the body, so the body is already partially filled in.
		size_t toRead = contentSize - body.size();

		// If toRead is 0, this means the body has already been fully read when reading the headers.
		// So no additional reading is required.
		while (toRead != 0)
		{
			size_t currentReadBody = body.size() - 1;
			body.resize(contentSize);
			int64 numRead = readFunction(CHUNK_SIZE, &body[currentReadBody]);
			if (numRead < 0)
				return HttpResponse::HttpError(std::format("Failed to read a response from the socket: {}", strerror(errno)));
			if (numRead == 0)
				break;
			toRead -= numRead;
		}
		return new HttpResponse(headers, body);
	}

	auto readChunkHeader = [&readFunction]() -> int64 {
		string chunkHeader;
		uByte last = 0;

		while (true)
		{
			uByte c = 0;
			auto read = readFunction(1, &c);

			if (read <= 0)
			{
				break;
			}

			if (c == '\n' && last == '\r')
			{
				break;
			}
			else if (c == '\r')
			{
				last = c;
				continue;
			}
			last = c;
			chunkHeader.push_back(c);
		}
		return std::stol(chunkHeader, 0, 16);
	};

	size_t toRead = 0;
	if (body.empty())
	{
		toRead = readChunkHeader();
	}
	else
	{
		size_t bufferPosition = 0;
		while (toRead < body.size() - bufferPosition)
		{
			bufferPosition += toRead;
			size_t readStartPosition = bufferPosition;
			string chunkHeader;
			while (true)
			{
				uByte c = body[bufferPosition++];
				if (c == '\n' && last == '\r')
				{
					break;
				}
				else if (c == '\r')
				{
					last = c;
					continue;
				}
				last = c;
				chunkHeader.push_back(c);
			}
			body.erase(body.begin() + readStartPosition, body.begin() + bufferPosition);
			toRead = std::stoul(chunkHeader, 0, 16) - body.size();
		}
	}

	size_t readPosition = body.size();
	while (true)
	{
		while (toRead != 0)
		{
			body.resize(readPosition + toRead);
			int64 numRead = readFunction(toRead, &body[readPosition]);
			readPosition += numRead;

			if (numRead < 0)
				return HttpResponse::HttpError(std::format("Failed to read a response from the socket: {}", strerror(errno)));
			if (numRead == 0)
				break;
			toRead -= numRead;
		}

		// Read chunk end
		std::array<uByte, 2> chunkEnd = { 0, 0 };

		readFunction(2, chunkEnd.data());

		if (chunkEnd != std::array<uByte, 2>{ '\r', '\n' })
		{
			return HttpResponse::HttpError(std::format("Expected an and of chunk (\\r \\n), got: {} {}", chunkEnd[0], chunkEnd[1]));
		}

		toRead = readChunkHeader();

		if (toRead == 0)
		{
			break;
		}
	}

	return new HttpResponse(headers, body);
}

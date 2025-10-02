#include "Http.h"
#include "Http.h"
#include <Core/Log.h>
#include <sstream>
#include <format>
#include <fstream>

using namespace engine;

using std::size_t;

http::HttpUrl::HttpUrl(string fromString)
{
	const std::vector<string> possibleUris = { SCHEME_HTTP, SCHEME_HTTPS };
	string uriType;

	for (auto& i : possibleUris)
	{
		if (fromString.substr(0, i.size()) == i)
		{
			uriType = i;
			fromString = fromString.substr(i.size());
			break;
		}
	}

	if (uriType.empty())
	{
		uriType = SCHEME_HTTP;
	}

	size_t slash = fromString.find_first_of("/");
	if (slash != string::npos)
	{
		this->Path = fromString.substr(slash);
		fromString = fromString.substr(0, slash);
	}
	else
	{
		this->Path = "/";
	}

	size_t colon = fromString.find_first_of(":");
	if (colon != string::npos)
	{
		this->Port = std::stoi(fromString.substr(colon + 1));
		fromString = fromString.substr(0, colon);
	}
	else
	{
		this->Port = uriType == SCHEME_HTTPS ? 443 : 80;
	}
	this->Scheme = uriType;
	this->HostName = fromString;
}

engine::http::HttpUrl::HttpUrl()
{
}

http::HttpResponse* http::SendRequest(string url, string method)
{
	return SendRequest(url, method,
		HttpOptions{

		});
}

http::HttpResponse* http::SendRequest(string url, string method, HttpOptions options)
{
	HttpUrl urlInfo = url;
	HttpResponse* response = nullptr;
#ifdef AIO_WITH_OPENSSL
	if (urlInfo.scheme == HttpUrl::SCHEME_HTTPS)
	{
		response = sendRequestSSL(urlInfo, method, options);
	}
	else
#endif
	{
		response = SendRequestNoSSL(urlInfo, method, options);
	}
	if (response->Status == HttpStatus::MovedPermanently || response->Status == HttpStatus::PermanentRedirect)
	{
		auto newLocation = response->Headers.find("location");

		// no location header specified, can't redirect
		if (newLocation == response->Headers.end())
		{
			return response;
		}

		Log::Note(str::Format("redirect: %s -> %s", url.c_str(), newLocation->second.c_str()));
		return SendRequest(newLocation->second, method, options);
	}
	return response;
}

string http::HttpOptions::GetHeadersString(HttpUrl target, string method) const
{
	std::stringstream messageStream;

	messageStream << method << " " << target.Path << " HTTP/1.1\r\n";
	messageStream << "Host: " << target.HostName << "\r\n";
	for (auto& i : Headers)
	{
		messageStream << i.first << ": " << i.second << "\r\n";
	}
	messageStream << "Connection: close\r\n";
	messageStream << "\r\n";
	return messageStream.str();
}

engine::http::HttpResponse::HttpResponse(const std::vector<uByte>& headerData, const std::vector<uByte>& content)
{
	auto Parsed = ParseHeaders(headerData);

	this->Headers = Parsed.Headers;
	this->Status = Parsed.Status;

	this->Body = new BufferStream(content.data(), content.size());
}

engine::http::HttpResponse::HttpResponse()
{
}

engine::http::HttpResponse::HttpResponse(HttpStatus Status)
{
	this->Status = Status;
}

engine::http::HttpResponse::HttpResponse(string body)
{
	this->Body = new BufferStream((uByte*)body.data(), body.size());
	this->Status = HttpStatus::OK;
}

engine::http::HttpResponse::HttpResponse(string body, HttpStatus Status)
{
	this->Body = new BufferStream((uByte*)body.data(), body.size());
	this->Status = Status;
}

engine::http::HttpResponse::HttpResponse(IBinaryStream* body, HttpStatus Status)
{
	this->Body = body;
	this->Status = Status;
}

engine::http::HttpResponse* engine::http::HttpResponse::HttpError(string description)
{
	Log::Warn("HTTP error: " + description);
	HttpResponse newError;

	newError.Status = HttpStatus::InternalError;
	newError.Body = new BufferStream((const uByte*)description.data(), description.size());

	return new HttpResponse(newError);
}

http::HttpResponse::ParseHeaderData engine::http::HttpResponse::ParseHeaders(const std::vector<uByte>& headerData)
{
	std::map<string, string> Headers;
	HttpStatus Status = HttpStatus::InternalError;
	string currentHeader;

	uByte last = 0;
	bool isFirstHeader = true;
	for (uByte b : headerData)
	{
		if (b == 0)
			continue;

		if (b == '\n' && last == '\r')
		{
			// Remove the newline
			currentHeader.pop_back();

			// Empty line means the content is next
			if (currentHeader.empty())
				break;

			if (isFirstHeader)
			{
				auto splitValues = str::Split(currentHeader, " ");

				if (splitValues.size() < 3 || (splitValues[0] != "HTTP/1.1" && splitValues[0] != "HTTP/1.0"))
				{
					return {};
				}

				Status = HttpStatus(std::atoi(splitValues[1].c_str()));

				isFirstHeader = false;
				currentHeader.clear();
				continue;
			}

			Headers.insert(ParseHeader(currentHeader));
			currentHeader.clear();
			continue;
		}
		last = b;
		currentHeader.push_back(b);
	}
	return ParseHeaderData{
		.Headers = Headers,
		.Status = Status,
	};
}

std::pair<string, string> engine::http::HttpResponse::ParseHeader(string fullHeader)
{
	size_t colon = fullHeader.find_first_of(':');

	if (colon == string::npos)
		return {str::Trim(fullHeader), "" };

	string name = str::Trim(fullHeader.substr(0, colon)), value = str::Trim(fullHeader.substr(colon + 1));
	return { name, value };
}

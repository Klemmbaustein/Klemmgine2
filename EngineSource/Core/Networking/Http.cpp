#include "Http.h"
#include "Http.h"
#include <Core/Log.h>
#include <sstream>
#include <format>
#include <fstream>

using namespace engine;

using std::size_t;

http::HttpUrl::HttpUrl(string FromString)
{
	const std::vector<string> PossibleUris = { SCHEME_HTTP, SCHEME_HTTPS };
	string UriType;

	for (auto& i : PossibleUris)
	{
		if (FromString.substr(0, i.size()) == i)
		{
			UriType = i;
			FromString = FromString.substr(i.size());
			break;
		}
	}

	if (UriType.empty())
	{
		UriType = SCHEME_HTTP;
	}

	size_t slash = FromString.find_first_of("/");
	if (slash != string::npos)
	{
		this->Path = FromString.substr(slash);
		FromString = FromString.substr(0, slash);
	}
	else
	{
		this->Path = "/";
	}

	size_t Colon = FromString.find_first_of(":");
	if (Colon != string::npos)
	{
		this->Port = std::stoi(FromString.substr(Colon + 1));
		FromString = FromString.substr(0, Colon);
	}
	else
	{
		this->Port = UriType == SCHEME_HTTPS ? 443 : 80;
	}
	this->Scheme = UriType;
	this->HostName = FromString;
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

http::HttpResponse* http::SendRequest(string Url, string Method, HttpOptions Options)
{
	HttpUrl UrlInfo = Url;
	HttpResponse* Response = nullptr;
#ifdef ENGINE_WITH_OPENSSL
	if (UrlInfo.scheme == HttpUrl::SCHEME_HTTPS)
	{
		Response = sendRequestSSL(UrlInfo, Method, Options);
	}
	else
#endif
	{
		Response = SendRequestNoSSL(UrlInfo, Method, Options);
	}
	if (Response->Status == HttpStatus::MovedPermanently || Response->Status == HttpStatus::PermanentRedirect)
	{
		auto NewLocation = Response->Headers.find("location");

		// no location header specified, can't redirect
		if (NewLocation == Response->Headers.end())
		{
			return Response;
		}

		Log::Note(str::Format("redirect: %s -> %s", Url.c_str(), NewLocation->second.c_str()));
		return SendRequest(NewLocation->second, Method, Options);
	}
	return Response;
}

string http::HttpOptions::GetHeadersString(HttpUrl Target, string Method) const
{
	std::stringstream MessageStream;

	MessageStream << Method << " " << Target.Path << " HTTP/1.1\r\n";
	MessageStream << "Host: " << Target.HostName << "\r\n";
	for (auto& i : Headers)
	{
		MessageStream << i.first << ": " << i.second << "\r\n";
	}
	MessageStream << "Connection: close\r\n";
	MessageStream << "\r\n";
	return MessageStream.str();
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

engine::http::HttpResponse::HttpResponse(string Body)
{
	this->Body = new BufferStream((uByte*)Body.data(), Body.size());
	this->Status = HttpStatus::OK;
}

engine::http::HttpResponse::HttpResponse(string Body, HttpStatus Status)
{
	this->Body = new BufferStream((uByte*)Body.data(), Body.size());
	this->Status = Status;
}

engine::http::HttpResponse::HttpResponse(IBinaryStream* Body, HttpStatus Status)
{
	this->Body = Body;
	this->Status = Status;
}

engine::http::HttpResponse* engine::http::HttpResponse::HttpError(string description)
{
	Log::Warn("HTTP error: " + description);
	HttpResponse NewError;

	NewError.Status = HttpStatus::InternalError;
	NewError.Body = new BufferStream((const uByte*)description.data(), description.size());

	return new HttpResponse(NewError);
}

http::HttpResponse::ParseHeaderData engine::http::HttpResponse::ParseHeaders(const std::vector<uByte>& headerData)
{
	std::map<string, string> Headers;
	HttpStatus Status = HttpStatus::InternalError;
	string CurrentHeader;

	uByte Last = 0;
	bool IsFirstHeader = true;
	for (uByte b : headerData)
	{
		if (b == 0)
			continue;

		if (b == '\n' && Last == '\r')
		{
			// Remove the newline
			CurrentHeader.pop_back();

			// Empty line means the content is next
			if (CurrentHeader.empty())
				break;

			if (IsFirstHeader)
			{
				auto SplitValues = str::Split(CurrentHeader, " ");

				if (SplitValues.size() < 3 || (SplitValues[0] != "HTTP/1.1" && SplitValues[0] != "HTTP/1.0"))
				{
					return {};
				}

				Status = HttpStatus(std::atoi(SplitValues[1].c_str()));

				IsFirstHeader = false;
				CurrentHeader.clear();
				continue;
			}

			Headers.insert(ParseHeader(CurrentHeader));
			CurrentHeader.clear();
			continue;
		}
		Last = b;
		CurrentHeader.push_back(b);
	}
	return ParseHeaderData{
		.Headers = Headers,
		.Status = Status,
	};
}

std::pair<string, string> engine::http::HttpResponse::ParseHeader(string FullHeader)
{
	size_t Colon = FullHeader.find_first_of(':');

	if (Colon == string::npos)
		return {str::Trim(FullHeader), "" };

	string Name = str::Trim(FullHeader.substr(0, Colon)), Value = str::Trim(FullHeader.substr(Colon + 1));
	return { Name, Value };
}

engine::http::HttpException::HttpException(const string& Message)
{
	this->Message = str::Format("Http error: %s", Message.c_str());
}

const char* engine::http::HttpException::what() const noexcept
{
	return Message.c_str();
}

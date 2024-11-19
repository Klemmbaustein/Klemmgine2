#pragma once
#include <vector>
#include <string>

namespace engine
{
	using string = std::string;

	namespace str
	{
		std::vector<string> Split(string Target, const string& Delim);
		string ReplaceChar(string Target, char c, char With);

		string Format(string Format, ...);
	}
}
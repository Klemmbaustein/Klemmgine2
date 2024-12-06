#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace engine
{
	using string = std::string;

	namespace str
	{
		std::vector<string> Split(string Target, const string& Delim);
		string ReplaceChar(string Target, char c, char With);

		string Lower(string Input);

		int32_t Hash(const string& Target);

		string Format(string Format, ...);
	}
}
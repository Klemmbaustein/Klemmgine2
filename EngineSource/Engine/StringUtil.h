#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace engine
{
	using string = std::string;

	namespace str
	{
		[[nodiscard]]
		std::vector<string> Split(string Target, const string& Delim);
		[[nodiscard]]
		string ReplaceChar(string Target, char c, char With);

		[[nodiscard]]
		string Lower(string Input);

		[[nodiscard]]
		string Trim(string Input);

		[[nodiscard]]
		int32_t Hash(const string& Target);

		[[nodiscard]]
		string Format(string Format, ...);
	}
}
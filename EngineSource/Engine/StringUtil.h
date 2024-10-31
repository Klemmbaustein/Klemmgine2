#pragma once
#include <vector>
#include <string>

namespace engine
{
	using string = std::string;

	namespace str
	{
		std::vector<std::string> Split(string Target, const string& Delim);
	}
}
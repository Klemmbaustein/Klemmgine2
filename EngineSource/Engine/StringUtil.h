#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace engine
{
	/// Alias for engine strings. An engine string is just an STL string.
	using string = std::string;

	namespace str
	{
		/**
		* @brief
		* Splits a string into substrings.
		* 
		* @param Target
		* the string
		*/
		[[nodiscard]]
		std::vector<string> Split(string Target, const string& Delim);
		[[nodiscard]]
		string ReplaceChar(string Target, char c, char With);

		/**
		* @brief
		* Returns the given string where all characters have been converted to lower case.
		*/
		[[nodiscard]]
		string Lower(string Input);

		/**
		* @brief
		* Removes whitespace from the start and end of the string.
		*/
		[[nodiscard]]
		string Trim(string Input);

		/**
		* @brief
		* Hashes the string, gives an unique integer from a string.
		* 
		* This function is used for object IDs. The object ID is derived from the object's name.
		*/
		[[nodiscard]]
		int32_t Hash(const string& Target);

		/**
		* @brief
		* Formats a string using the C printf functions.
		* 
		* Any string passed with %s must be passed as a C-string!
		*/
		[[nodiscard]]
		string Format(string Format, ...);
	}
}
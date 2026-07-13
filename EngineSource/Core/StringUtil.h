#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace engine
{
	/// Alias for engine strings. An engine string is just an STL string.
	using string = std::string;

	/**
	* @brief
	* Engine string utility functions.
	*
	* Contains various utility functions related to strings.
	*
	* @ingroup engine-core
	*/
	namespace str
	{
		/**
		* @brief
		* Splits a string into substrings.
		*
		* @param Target
		* The input string that should be split into chunks.
		*
		* @param Delim
		* A string containing all characters that function as delimiters.
		*
		* @return
		* The a vector containing the substrings.
		* If a substring would be empty (if 2 delimiters were read one after another),
		* It won't be added to the result.
		*/
		[[nodiscard]]
		std::vector<string> Split(string Target, const string& Delim);

		/**
		* @brief
		* Returns a string where all occurrences of the character 'c' have
		* been replaced with the character 'With'
		*/
		[[nodiscard]]
		string ReplaceChar(string Target, char c, char With);
		/**
		* @brief
		* Returns a string where all occurrences of the character 'c' are removed
		*/
		[[nodiscard]]
		string RemoveChar(string Target, char c);

		[[nodiscard]]
		string Replace(string Target, string Substr, string Replacement);

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
		constexpr int32_t Hash(const string& Target)
		{
			unsigned int hash = 1315423911;

			for (size_t i = 0; i < Target.size(); i++)
			{
				hash ^= ((hash << 5) + Target[i] + (hash >> 2));
			}

			return (hash & 0x7FFFFFFF);
		}

		/**
		* @brief
		* Formats a string using the C printf functions.
		*
		* Any string passed with %s must be passed as a C-string!
		*/
		[[nodiscard]]
		string Format(string Format, ...);

		constexpr string AddSpacesToName(const string& Target)
		{
			bool LastWasLower = false;

			string Result;
			Result.reserve(Target.size());

			for (auto& i : Target)
			{
				if (std::isupper(i) && LastWasLower)
				{
					LastWasLower = false;
					Result.push_back(' ');
				}
				else if (std::islower(i))
				{
					LastWasLower = true;
				}
				Result.push_back(i);
			}
			return Result;
		}

		string FloatToString(float Val, size_t Precision = 0);
	}
}
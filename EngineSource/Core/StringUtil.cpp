#include "StringUtil.h"
#include "StringUtil.h"
#include <iostream>
#include <cstdarg>
#include <cmath>
#include <algorithm>

std::vector<engine::string> engine::str::Split(string Target, const string& Delim)
{
	std::vector<std::string> Out;

	size_t FirstDelim = Target.find_first_of(Delim);
	while (FirstDelim != std::string::npos)
	{
		string SplitPart = Target.substr(0, FirstDelim);
		if (!SplitPart.empty())
			Out.push_back(SplitPart);

		Target = Target.substr(FirstDelim + 1);

		FirstDelim = Target.find_first_of(Delim);
	}

	if (!Target.empty())
		Out.push_back(Target);

	return Out;
}

engine::string engine::str::ReplaceChar(string Target, char c, char With)
{
	for (char& it : Target)
	{
		if (it == c)
		{
			it = With;
		}
	}
	return Target;
}

engine::string engine::str::Lower(string Input)
{
	std::transform(Input.begin(), Input.end(), Input.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return Input;
}

engine::string engine::str::Trim(string Input)
{
	Input.erase(Input.begin(), std::find_if(Input.begin(), Input.end(), [](unsigned char ch) {
		return !std::isspace(ch) && ch != '\r';
		}));

	Input.erase(std::find_if(Input.rbegin(), Input.rend(), [](unsigned char ch) {
		return !std::isspace(ch) && ch != '\r';
		}).base(), Input.end());
	return Input;
}

int32_t engine::str::Hash(const string& Target)
{
	unsigned int hash = 1315423911;

	for (size_t i = 0; i < Target.size(); i++)
	{
		hash ^= ((hash << 5) + Target[i] + (hash >> 2));
	}

	return (hash & 0x7FFFFFFF);
}

engine::string engine::str::Format(string Format, ...)
{
	size_t Size = Format.size() + 50, NewSize = Size;
	char* Buffer = nullptr;
	do
	{
		Size = NewSize;
		if (Buffer)
		{
			delete[] Buffer;
		}
		Buffer = new char[Size + 1]();
		va_list va;
		va_start(va, Format);
		NewSize = vsnprintf(Buffer, Size + 1, Format.c_str(), va);
		va_end(va);

	} while (NewSize > Size);


	string StrBuffer = Buffer;
	delete[] Buffer;
	return StrBuffer;
}

engine::string engine::str::FloatToString(float Val, size_t Precision)
{
	string Out = std::to_string(Val);

	while (Out.size()
		&& (Out[Out.size() - 1] == '0' || Out[Out.size() - 1] == '.'))
	{
		if (Out[Out.size() - 1] == '.')
		{
			Out.pop_back();
			break;
		}
		Out.pop_back();
	}

	if (Precision == 0)
	{
		return Out;
	}

	size_t Dot = Out.find_first_of('.');

	return Out.substr(0, Dot + 1 + Precision);
}
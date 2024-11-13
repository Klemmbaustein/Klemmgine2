#include "StringUtil.h"
#include <iostream>

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

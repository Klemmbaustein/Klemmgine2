#include "LaunchArgs.h"
#include <map>
#include <Engine/Log.h>

namespace engine::launchArgs
{
	std::map<string, std::vector<Parameter>> LaunchParameters;
}

using namespace engine::launchArgs;
using namespace engine;

void engine::launchArgs::SetArgs(int argc, char** argv)
{
	string NewName;
	std::vector<Parameter> New;

	for (int i = 1; i < argc; i++)
	{
		string Arg = argv[i];

		if (Arg.empty())
			continue;

		if (Arg[0] != '-')
		{
			if (NewName.empty())
			{
				Log::Error("Expected first argument to start with '-'");
				continue;
			}
			else
			{
				New.push_back(Parameter{
					.Value = Arg
					});
			}
		}
		else
		{
			if (!NewName.empty())
			{
				LaunchParameters.insert({NewName, New});
				New.clear();
			}
			NewName = Arg.substr(1);
		}
	}
	if (!NewName.empty())
	{
		LaunchParameters.insert({ NewName, New });
		New.clear();
	}
}

int32 Parameter::AsInt() const
{
	try
	{
		return std::stoi(Value);
	}
	catch (std::invalid_argument)
	{
		return 0;
	}
}

std::optional<std::vector<Parameter>> engine::launchArgs::GetArg(string Name)
{
	if (LaunchParameters.contains(Name))
	{
		return LaunchParameters.at(Name);
	}
	return {};
}

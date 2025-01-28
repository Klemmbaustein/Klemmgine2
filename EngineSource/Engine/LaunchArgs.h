#pragma once
#include <Engine/Types.h>
#include <Engine/File/AssetRef.h>
#include <optional>
#include <vector>

namespace engine::launchArgs
{
	void SetArgs(int argc, char** argv);

	struct Parameter
	{
		string Value;

		string AsString() const;
		AssetRef AsFile();
		int32 AsInt() const;
		float AsFloat() const;
	};

	std::optional<std::vector<Parameter>> GetArg(string Name);
}
#pragma once
#include <Core/Types.h>
#include <optional>

namespace engine::editor
{
#ifdef EDITOR
	const bool IsActive();

	string GetEditorPath();

	void OpenEditorAt(string Path);

	std::optional<string> GetRemoteProjectName();

	void SetRemoteProject(string Path);
	void ClearRemoteProject();

#else
	constexpr bool IsActive() { return false; }
#endif
}
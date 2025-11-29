#pragma once
#include <Core/Types.h>

namespace engine::editor
{
#ifdef EDITOR
	const bool IsActive();

	string GetEditorPath();

	void OpenEditorAt(string Path);
#else
	constexpr bool IsActive() { return false; }
#endif
}
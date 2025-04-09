#pragma once

namespace engine::editor
{
#ifdef EDITOR
	const bool IsActive();
#else
	constexpr bool IsActive() { return false; }
#endif
}
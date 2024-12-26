#pragma once

namespace engine::editor
{
#ifdef EDITOR
	const bool IsActive();
#else
	consteval bool IsActive() { return false; }
#endif
}
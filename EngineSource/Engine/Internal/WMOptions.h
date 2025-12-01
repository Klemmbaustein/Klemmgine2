#pragma once
#include <kui/Window.h>

namespace engine
{
	/**
	* @brief
	* Extended KlemmUI window flags used by the game engine's windowing code.
	*/
	enum class EngineWindowFlag : int
	{
		/// The window is the game engine window displaying the 3D scene.
		EngineWindow = 1 << 16,
	};
}
inline kui::Window::WindowFlag operator|(kui::Window::WindowFlag a, engine::EngineWindowFlag b)
{
	return kui::Window::WindowFlag(int(a) | int(b));
}

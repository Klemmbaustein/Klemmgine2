#pragma once
#include <SDL3/SDL.h>
#include <mutex>
#include <vector>
#include <kui/Window.h>
#include <array>

namespace kui::systemWM
{
	/**
	* @brief
	* Internal KlemmUI Window implementation using SDL.
	*/
	class SysWindow
	{
		// @cond
	public:
		bool IsMain = false;
		SDL_Window* SDLWindow = nullptr;
		Window* Parent = nullptr;
		SDL_GLContext GLContext;

		std::array<SDL_Cursor*, size_t(Window::Cursor::End)> WindowCursors;

		std::string TextInput;

		std::vector<SDL_Event> Events;

		void HandleKey(SDL_Keycode k, bool IsDown);

		void UpdateEvents();
		// @endcond
	};
}
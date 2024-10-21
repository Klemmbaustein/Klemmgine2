#pragma once
#include <SDL3/SDL.h>
#include <mutex>
#include <vector>
#include <kui/Window.h>

namespace kui::systemWM
{
	class SysWindow
	{
	public:
		SDL_Window* SDLWindow = nullptr;
		Window* Parent = nullptr;
		SDL_GLContext GLContext;

		std::vector<SDL_Event> Events;

		void UpdateEvents();
	};
}
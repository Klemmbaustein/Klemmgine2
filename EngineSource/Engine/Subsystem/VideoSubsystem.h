#pragma once
#include "Subsystem.h"
#include <kui/Window.h>

namespace engine::subsystem
{
	class VideoSubsystem : public Subsystem
	{
	public:
		VideoSubsystem();
		virtual ~VideoSubsystem() override;

		virtual void Update() override;
		virtual void RenderUpdate() override;

		kui::Window* MainWindow = nullptr;

		std::vector<std::function<void(kui::Vec2ui NewSize)>> OnResizedCallbacks;

	private:
		void OnResized();
	};
}
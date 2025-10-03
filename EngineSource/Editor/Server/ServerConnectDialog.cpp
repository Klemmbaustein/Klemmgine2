#include "ServerConnectDialog.h"
#include <kui/Window.h>
#include <SDL3/SDL.h>
#include <DialogWindow.kui.hpp>
#include <condition_variable>
#include <mutex>
using namespace engine::editor;
using namespace kui;

ConnectResult engine::editor::ServerConnectDialog::Show()
{
	bool ConnectionAccepted = false;

	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
	bool Accepted = false;
	{
		Window win = Window("Connect to server...", Window::WindowFlag::None, Window::POSITION_CENTERED, Vec2ui(400, 300));

		Font* fnt = new Font("res:DefaultFont.ttf");
		win.Markup.SetDefaultFont(fnt);

		UIBox* bx = new UIBox(true, -1);

		bx->SetMinSize(2);
		bx
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetHorizontalAlign(UIBox::Align::Centered);

		auto btn = new DialogWindowButton();
		btn->SetText("Connect!");
		btn->btn->OnClicked = [&] { Accepted = true; win.Close(); };
		bx->AddChild(btn);

		while (win.UpdateWindow())
		{

		}

		delete fnt;
	}

	if (!Accepted)
	{
		return ConnectResult{
			.Connect = false,
		};
	}

	auto c = new ServerConnection("localhost:7274");
	std::condition_variable cv;
	std::mutex m;
	bool GotResult = false;

	c->OnConnectionAcceptDeny = [&](bool Accept){
		ConnectionAccepted = Accept;
		GotResult = true;
		cv.notify_one();
	};

	std::unique_lock lk(m);
	cv.wait(lk, [&] { return GotResult; });

	if (!ConnectionAccepted)
	{
		delete c;
		c = nullptr;
	}

	return ConnectResult{
		.Connect = ConnectionAccepted,
		.Connection = c,
	};
}

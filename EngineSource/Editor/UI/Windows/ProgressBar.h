#ifdef EDITOR
#pragma once
#include "IPopupWindow.h"
#include <LoadingBar.kui.hpp>
#include <mutex>

namespace engine::editor
{
	class ProgressBar : public IPopupWindow
	{
	public:
		ProgressBar(string Title);

		LoadingBarElement* Elem = nullptr;

		std::atomic<float> Progress = 0;

		void Begin() override;
		void Update() override;
		void Destroy() override;

		void SetMessage(string NewMessage);

	private:
		float Time = 0;
		bool UpdateTitle = false;
		string ProgressMessage;
		std::mutex ProgressMessageMutex;
	};
}
#endif
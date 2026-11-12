#pragma once
#include <vector>
#include <kui/UI/UIBox.h>

namespace engine
{
	class Popup
	{
	public:

		Popup(bool Clear = true);
		virtual ~Popup();

		kui::UIBox* Box = nullptr;

		void SetBox(kui::UIBox* NewBox);

		static void Clear(size_t ToDepth = 0);
		static void UpdatePopups();
		static thread_local std::vector<Popup*> Current;
	};
}
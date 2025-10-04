#pragma once
#include <kui/UI/UIBox.h>
#include <Engine/Engine.h>
#include <Engine/Graphics/VideoSubsystem.h>

namespace engine
{
	class Scene;

	class UICanvas
	{
	protected:

		UICanvas();

	public:

		template<typename T>
		static T* CreateNew()
		{
			if (!Engine::IsPlaying || !VideoSubsystem::Current)
				return nullptr;
			T* New = new T();
			RegisterSelf(New);
			return New;
		}

		virtual ~UICanvas();
		virtual void Update();

		kui::UIBox* CanvasBox = nullptr;

		static void UpdateAll();
		static void ClearAll();

	private:

		static std::vector<UICanvas*> ActiveCanvases;
		static void RegisterSelf(UICanvas* This);
	};
}
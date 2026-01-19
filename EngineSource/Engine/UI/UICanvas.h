#pragma once
#include <kui/UI/UIBox.h>
#include <kui/UI/UICanvasBox.h>
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
			return New;
		}

		virtual ~UICanvas();
		virtual void Update();

		kui::UICanvasBox* CanvasBox = nullptr;

		static void UpdateAll();
		static void ClearAll();

	private:

		static std::vector<UICanvas*> ActiveCanvases;
		static void RegisterSelf(UICanvas* This);
	};
}
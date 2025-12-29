#pragma once
#include "IDialogWindow.h"
#include <kui/UI/UITextField.h>
#include <array>
#include <Core/Vector.h>

namespace engine::editor
{
	class ColorPicker : public IDialogWindow
	{
	public:

		struct HsvColor
		{
			uByte h;
			uByte s;
			uByte v;
		};


		ColorPicker(Vector3 RgbColorValue, std::function<void(Vector3)> OnSubmit,
			std::function<void(Vector3)> OnChange);

		void Begin() override;

		void Update() override;

		// Inherited via IDialogWindow
		void Destroy() override;

		void UpdateColor(bool UpdateFields = true);

		kui::UIBackground* PreviewColor = nullptr;

		kui::Shader* HueShader = nullptr;
		kui::Shader* PickerShader = nullptr;

		kui::UIBackground* PickerBackground = nullptr;
		kui::UIBackground* HueBackground = nullptr;

		HsvColor SelectedHsv{255};

		bool DraggingPicker = false;
		bool DraggingHue = false;
		Vector3 RgbColorValue;
		Vector3 InitialValue;

		std::function<void(Vector3)> OnSubmit;
		std::function<void(Vector3)> OnChange;

		std::array<kui::UITextField*, 3> PartFields = {};
	};
}
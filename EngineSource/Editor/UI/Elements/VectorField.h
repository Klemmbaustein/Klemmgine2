#ifdef EDITOR
#pragma once
#include <kui/UI/UIBox.h>
#include <kui/UI/UITextField.h>
#include <Core/Vector.h>
#include <functional>
#include <array>

namespace engine::editor
{
	class VectorField : public kui::UIBox
	{
	public:
		std::function<void()> OnChanged;

		VectorField(Vector3 InitialValue, kui::UISize Size, std::function<void()> OnChanged);
		virtual ~VectorField() override;

		Vector3 GetValue() const;
		void SetValue(Vector3 NewValue);

		void UpdateValues();

	private:
		std::array<kui::UITextField*, 3> TextFields;
		Vector3 Value;
	};
}
#endif
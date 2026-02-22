#pragma once
#include <Core/File/SerializedData.h>
#include <Engine/Script/UI/ParseUI.h>

namespace engine::script::ui
{
	void SerializeUIElement(kui::markup::UIElement* Data, std::vector<SerializedData>& OutValues);
	void SerializeUIProperties(const std::vector<kui::markup::Property>& Properties,
		std::vector<SerializedData>& OutValues);

	void SerializeUI(UIParseData* Data, std::vector<SerializedData>& OutValues);

	void DeSerializeUIElement(kui::markup::UIElement& ToElement, SerializedValue& FromValues);
	void DeSerializeUIProperties(std::vector<kui::markup::Property>& Properties, SerializedValue& FromValues);

	UIParseData DeSerializeUI(SerializedValue& FromValues);
}
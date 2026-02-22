#include "UISerializer.h"

using namespace kui::stringParse;
using namespace kui::markup;
using namespace engine::script::ui;

void engine::script::ui::SerializeUIElement(kui::markup::UIElement* Data, std::vector<SerializedData>& OutValues)
{
	OutValues.push_back(SerializedData("name", Data->ElementName.Text));
	OutValues.push_back(SerializedData("typeName", Data->TypeName.Text));
	OutValues.push_back(SerializedData("type", uByte(Data->Type)));
	std::vector<SerializedValue> Children;

	for (auto& i : Data->Children)
	{
		std::vector<SerializedData> Element;
		SerializeUIElement(&i, Element);
		Children.push_back(Element);
	}

	std::vector<SerializedData> Properties;
	SerializeUIProperties(Data->ElementProperties, Properties);
	std::vector<SerializedData> GlobalProperties;
	SerializeUIProperties(Data->GlobalProperties, GlobalProperties);
	std::vector<SerializedData> TranslatedProperties;
	SerializeUIProperties(Data->TranslatedProperties, TranslatedProperties);
	OutValues.push_back(SerializedData("props", Properties));
	OutValues.push_back(SerializedData("globalProps", GlobalProperties));
	OutValues.push_back(SerializedData("translatedProps", TranslatedProperties));
	OutValues.push_back(SerializedData("children", Children));

	std::vector<SerializedData> Variables;
	for (auto& i : Data->Variables)
	{
		Variables.push_back(SerializedData(i.first, SerializedValue(std::vector{
			SerializedData("name", i.second.Token.Text),
			SerializedData("type", int32(i.second.Type)),
			SerializedData("value", i.second.Value),
			})));
	}
}

void engine::script::ui::SerializeUIProperties(const std::vector<kui::markup::Property>& Properties,
	std::vector<SerializedData>& OutValues)
{
	for (auto& i : Properties)
	{
		OutValues.push_back(SerializedData(i.Name.Text, i.Value.Text));
	}
}

void engine::script::ui::SerializeUI(UIParseData* Data, std::vector<SerializedData>& OutValues)
{
	std::vector<SerializedValue> Elements;

	for (auto& i : Data->UIData.Elements)
	{
		std::vector<SerializedData> Element;
		SerializeUIElement(&i.Root, Element);

		Elements.push_back(SerializedValue(std::vector{
			SerializedData("name", i.FromToken.Text),
			SerializedData("root", Element),
			}));
	}

	OutValues.push_back(SerializedData("elems", Elements));
}

void engine::script::ui::DeSerializeUIElement(kui::markup::UIElement& ToElement, SerializedValue& FromValues)
{
	ToElement.ElementName = StringToken(FromValues.At("name").GetString(), 0, 0);
	ToElement.TypeName = StringToken(FromValues.At("typeName").GetString(), 0, 0);
	ToElement.Type = UIElement::ElementType(FromValues.At("type").GetByte());

	DeSerializeUIProperties(ToElement.ElementProperties, FromValues.At("props"));
	DeSerializeUIProperties(ToElement.GlobalProperties, FromValues.At("globalProps"));
	DeSerializeUIProperties(ToElement.TranslatedProperties, FromValues.At("translatedProps"));

	auto& Children = FromValues.At("children").GetArray();
	for (auto& i : Children)
	{
		DeSerializeUIElement(ToElement.Children.emplace_back(), i);
	}
}

void engine::script::ui::DeSerializeUIProperties(std::vector<kui::markup::Property>& Properties,
	SerializedValue& FromValues)
{
	for (auto& [Name, Value] : FromValues.GetObject())
	{
		Properties.push_back(Property(StringToken(Name, 0, 0), StringToken(Value.GetString(), 0, 0)));
	}
}

UIParseData engine::script::ui::DeSerializeUI(SerializedValue& FromValues)
{
	UIParseData OutData;

	auto& Elements = FromValues.At("elems").GetArray();

	for (auto& i : Elements)
	{
		UIElement Element;
		DeSerializeUIElement(Element, i.At("root"));

		OutData.UIData.Elements.push_back(MarkupElement{
			.Root = Element,
			.FromToken = StringToken(i.At("name").GetString(), 0, 0),
			});
	}

	return OutData;
}

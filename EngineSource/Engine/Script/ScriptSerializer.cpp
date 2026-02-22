#include "ScriptSerializer.h"
#include <Engine/Script/UI/UISerializer.h>
#include <Core/File/BinarySerializer.h>
using namespace ds;
using namespace engine;

static const string SCRIPT_FORMAT_ID = "k2script";

void script::serialize::SerializeBytecode(ds::BytecodeStream* Stream, ui::UIParseData* UIData, IBinaryStream* To)
{
	SerializedValue BytecodeValue = std::vector<SerializedValue>{};

	for (uByte i : Stream->code.buffer)
	{
		BytecodeValue.Append(i);
	}

	SerializedValue ExternFunctions = std::vector<SerializedValue>{};

	for (const string& i : Stream->externalFunctions)
	{
		ExternFunctions.Append(i);
	}

	SerializedValue ReflectData = std::vector<SerializedValue>{};

	for (auto& i : Stream->reflect.types)
	{
		SerializedValue Members = std::vector<SerializedData>{};

		for (auto& m : i.second.members)
		{
			Members.Append(SerializedData(m.name, SerializedValue({
				SerializedData("offset", int32(m.offset)),
				SerializedData("type", int32(m.type))
				})));
		}

		SerializedValue Interfaces = std::vector<SerializedValue>{};

		for (auto& interface : i.second.interfaces)
		{
			Interfaces.Append(SerializedValue(std::vector{
				SerializedValue(int32(interface.first)),
				SerializedValue(int32(interface.second)),
				}));
		}

		ReflectData.Append(SerializedValue({
			SerializedData("id", int32(i.second.hash)),
			SerializedData("name", i.second.name),
			SerializedData("construct", int32(i.second.constructor)),
			SerializedData("size", int32(i.second.bodySize)),
			SerializedData("vTable", int32(i.second.vTableOffset)),
			SerializedData("members", Members),
			SerializedData("super", int32(i.second.superClass)),
			SerializedData("interfaces", Interfaces),
			}));
	}

	SerializedValue VTable = std::vector<SerializedValue>{};

	for (const VTableFunction& i : Stream->virtualTable)
	{
		VTable.Append(int32(i.codeOffset));
		VTable.Append(int32(i.nativeFunction));
	}

	std::vector<SerializedData> UIObject;

	ui::SerializeUI(UIData, UIObject);

	SerializedValue BytecodeData = std::vector{
		SerializedData("code", BytecodeValue),
		SerializedData("extern", ExternFunctions),
		SerializedData("reflect", ReflectData),
		SerializedData("virtual", VTable),
		SerializedData("ui", UIObject),
	};

	BinarySerializer::ToBinaryData(BytecodeData.GetObject(), To, SCRIPT_FORMAT_ID);
}

void engine::script::serialize::DeSerializeBytecode(ds::BytecodeStream* ToStream, ui::UIParseData* UIData, IBinaryStream* FromStream)
{
	SerializedValue Obj = BinarySerializer::FromStream(FromStream, SCRIPT_FORMAT_ID);

	auto& CodeData = Obj.At("code").GetArray();
	ToStream->code.buffer.clear();
	for (auto& i : CodeData)
	{
		ToStream->code.buffer.push_back(i.GetByte());
	}

	auto& ExternFunctions = Obj.At("extern").GetArray();
	ToStream->externalFunctions.clear();
	for (auto& i : ExternFunctions)
	{
		ToStream->externalFunctions.push_back(i.GetString());
	}

	auto& VirtualData = Obj.At("virtual").GetArray();
	ToStream->virtualTable.clear();
	for (size_t i = 0; i < VirtualData.size(); i += 2)
	{
		ToStream->virtualTable.push_back(VTableFunction{
			.codeOffset = BytecodeOffset(VirtualData[i].GetInt()),
			.nativeFunction = uint32_t(VirtualData[i + 1].GetInt()),
			});
	}

	auto& ReflectData = Obj.At("reflect").GetArray();
	ToStream->reflect.types.clear();
	for (auto& i : ReflectData)
	{
		TypeInfo NewType;
		NewType.hash = i.At("id").GetInt();
		NewType.name = i.At("name").GetString();
		NewType.constructor = i.At("construct").GetInt();
		NewType.bodySize = i.At("size").GetInt();
		NewType.vTableOffset = i.At("vTable").GetInt();
		NewType.superClass = i.At("super").GetInt();
		auto& Interfaces = i.At("interfaces").GetArray();

		for (auto& interface : Interfaces)
		{
			auto& Entry = interface.GetArray();
			NewType.interfaces.insert({
				TypeId(interface.At(0).GetInt()),
				BytecodeOffset(interface.At(1).GetInt()) });
		}

		auto& Members = i.At("members").GetObject();

		for (auto& m : Members)
		{
			NewType.members.push_back(TypeMember{
				.type = TypeId(m.Value.At("type").GetInt()),
				.name = m.Name,
				.offset = BytecodeOffset(m.Value.At("offset").GetInt()),
				});
		}
		ToStream->reflect.types.insert({ NewType.hash, NewType });
	}

	*UIData = ui::DeSerializeUI(Obj.At("ui"));
}

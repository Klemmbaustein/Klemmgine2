#include "ScriptSerializer.h"
#include <Core/File/BinarySerializer.h>
using namespace ds;
using namespace engine;

static const string SCRIPT_FORMAT_ID = "k2script";

void script::serialize::SerializeBytecode(BytecodeStream* Stream, IBinaryStream* ToStream)
{
	SerializedValue BytecodeValue = std::vector<SerializedValue>{};

	for (uint8 i : Stream->code.buffer)
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

		ReflectData.Append(SerializedValue({
			SerializedData("id", int32(i.second.hash)),
			SerializedData("name", i.second.name),
			SerializedData("construct", int32(i.second.constructor)),
			SerializedData("size", int32(i.second.bodySize)),
			SerializedData("vTable", int32(i.second.vTableOffset)),
			SerializedData("members", Members),
			}));
	}

	SerializedValue VTable = std::vector<SerializedValue>{};

	for (const VTableEntry& i : Stream->virtualTable)
	{
		VTable.Append(int32(i.codeOffset));
	}

	SerializedValue BytecodeData = std::vector{
		SerializedData("code", BytecodeValue),
		SerializedData("extern", ExternFunctions),
		SerializedData("reflect", ReflectData),
		SerializedData("virtual", VTable),
	};

	BinarySerializer::ToBinaryData(BytecodeData.GetObject(), ToStream, SCRIPT_FORMAT_ID);
}

void engine::script::serialize::DeSerializeBytecode(ds::BytecodeStream* ToStream, IBinaryStream* FromStream)
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
	for (auto& i : VirtualData)
	{
		ToStream->virtualTable.push_back(VTableEntry(i.GetInt()));
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

		auto& Members = i.At("members").GetObject();

		for (auto& m : Members)
		{
			NewType.members.push_back(TypeMember{
				.type = TypeHash(m.Value.At("type").GetInt()),
				.name = m.Name,
				.offset = bytecodeOffset(m.Value.At("offset").GetInt()),
				});
		}
		ToStream->reflect.types.insert({ NewType.hash, NewType });
	}
}

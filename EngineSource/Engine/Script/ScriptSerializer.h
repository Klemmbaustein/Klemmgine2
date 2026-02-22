#pragma once
#include <ds/bytecode.hpp>
#include <Engine/Script/UI/ParseUI.h>
#include <Core/File/BinaryStream.h>

namespace engine::script::serialize
{
	void SerializeBytecode(ds::BytecodeStream* Stream, ui::UIParseData* UIData, IBinaryStream* To);
	void DeSerializeBytecode(ds::BytecodeStream* ToStream, ui::UIParseData* UIData, IBinaryStream* from);
}
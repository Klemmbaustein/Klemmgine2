#pragma once
#include <bytecode.hpp>
#include <Core/File/BinaryStream.h>

namespace engine::script::serialize
{
	void SerializeBytecode(lang::BytecodeStream* Stream, IBinaryStream* to);
	void DeSerializeBytecode(lang::BytecodeStream* ToStream, IBinaryStream* from);
}
#pragma once
#include <ds/bytecode.hpp>
#include <Core/File/BinaryStream.h>

namespace engine::script::serialize
{
	void SerializeBytecode(ds::BytecodeStream* Stream, IBinaryStream* to);
	void DeSerializeBytecode(ds::BytecodeStream* ToStream, IBinaryStream* from);
}
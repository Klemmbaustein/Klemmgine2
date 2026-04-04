#pragma once
#include <ds/bytecode.hpp>
#include <Engine/Script/UI/ParseUI.h>
#include <Core/File/BinaryStream.h>

namespace engine::script::serialize
{
	/**
	 * @brief
	 * Serializes the given bytecode stream and UI data structure to the given binary stream.
	 * @param Stream
	 * The compiled script data to write.
	 * @param UIData
	 * The compiled UI data to write.
	 * @param To
	 * The binary stream to write the data to.
	 */
	void SerializeBytecode(ds::BytecodeStream* Stream, ui::UIParseData* UIData, IBinaryStream* To);
	/**
	 * @brief
	 * De-serializes the given bytecode stream and UI data structure from the given binary stream.
	 * @param ToStream
	 * The bytecode data structure that should receive the de-serialized script bytecode data.
	 * @param UIData
	 * The UI data structure that should receive the de-serialized UI.
	 * @param FromStream
	 * The binary stream to write the data to.
	 */
	void DeSerializeBytecode(ds::BytecodeStream* ToStream, ui::UIParseData* UIData, IBinaryStream* FromStream);
}
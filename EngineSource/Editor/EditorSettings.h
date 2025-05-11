#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>

namespace engine::editor
{
	struct Settings : ISerializable
	{
		Settings();

		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;
	};
}
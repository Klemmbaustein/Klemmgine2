#pragma once
#include <ds/native/nativeModule.hpp>
#include <Core/File/SerializedData.h>

namespace engine::script
{

#define SCRIPT_SERIALIZED_VALUE_ID(x) constexpr static inline ds::TypeId ID = ds::typeIdFromName(x)

	struct ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedValue");
		SerializedData::DataType Type;
	};

	struct ScriptSerializedInt : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedInt");
		ds::Int IntValue;
	};

	struct ScriptSerializedFloat : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedFloat");
		ds::Float FloatValue;
	};

	struct ScriptSerializedString : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedString");
		ds::RuntimeClass* StringValue;
	};

	struct ScriptSerializedVector3 : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedVector3");
		Vector3 Vector3Value;
	};

	struct ScriptSerializedVector2 : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedVector2");
		Vector2 Vector2Value;
	};

	struct ScriptSerializedObject : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedObject");
		ds::RuntimeClass* KeyValueArray;
	};

	struct ScriptSerializedArray : ScriptSerializedValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedArray");
		ds::RuntimeClass* ValueArray;
	};

	struct ScriptSerializedKeyValue
	{
		SCRIPT_SERIALIZED_VALUE_ID("engine::serialize::SerializedKeyValue");

		ds::RuntimeClass* NameString;
		ds::RuntimeClass* Value;
	};

	struct SerializeBindings
	{
		ds::ClassType* SerializedKeyValue = nullptr;
		ds::ClassType* SerializedValue = nullptr;
	};

	SerializeBindings AddSerializeModule(ds::NativeModule& To, ds::LanguageContext* ToContext);

	ds::RuntimeClass* MarshalSerializedData(SerializedData& DataValue);
	ds::RuntimeClass* MarshalSerializedValue(SerializedValue& Value);

	SerializedData UnMarshalSerializedData(ds::RuntimeClass* cls);
	SerializedValue UnMarshalSerializedValue(ds::RuntimeClass* cls);
}
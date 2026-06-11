#include "SerializeBindings.h"
#include <ds/language.hpp>
#include <ds/modules/system.hpp>
#include <sstream>
#include <Core/File/TextSerializer.h>
#include <Core/File/JsonSerializer.h>
#include <ds/parser/types/arrayType.hpp>
#include <Core/Log.h>

using namespace engine::script;
using namespace engine;
using namespace ds;

static void SerializedKeyValue_delete(InterpretContext* context)
{
	auto Value = context->popPtr<ScriptSerializedKeyValue>();

	context->destruct(Value->NameString);
	context->destruct(Value->Value);
}

RuntimeFunction SerializedKeyValue_vTable = {
	.nativeFn = &SerializedKeyValue_delete
};

static void SerializedObject_delete(InterpretContext* context)
{
	auto Value = context->popPtr<ScriptSerializedObject>();

	context->destruct(Value->KeyValueArray);
}

RuntimeFunction SerializedObject_vTable = {
	.nativeFn = &SerializedObject_delete
};

static void Serialize_parseSerializedString(InterpretContext* context)
{
	auto str = context->popRuntimeString();

	std::stringstream Stream;

	Stream << str.ptr();

	try
	{
		auto Value = SerializedValue(TextSerializer::FromStream(Stream));
		context->pushValue(MarshalSerializedValue(Value));
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}

static void Serialize_parseJsonString(InterpretContext* context)
{
	auto str = context->popRuntimeString();

	std::stringstream Stream;

	Stream << str.ptr();

	try
	{
		auto Value = JsonSerializer::FromStream(Stream);
		context->pushValue(MarshalSerializedValue(Value));
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}
SerializeBindings engine::script::AddSerializeModule(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto IntInst = ToContext->registry->getEntry<IntType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();

	SerializeBindings out;

	NativeModule Serialize;

	Serialize.name = "engine::serialize";

	out.SerializedValue = Serialize.createClass<ScriptSerializedValue>("SerializedValue");
	auto ObjectValue = Serialize.createClass<ScriptSerializedObject>("SerializedObject", out.SerializedValue);
	auto IntValue = Serialize.createClass<ScriptSerializedInt>("SerializedInt", out.SerializedValue);
	auto FloatValue = Serialize.createClass<ScriptSerializedInt>("SerializedFloat", out.SerializedValue);
	out.SerializedKeyValue = Serialize.createClass<ScriptSerializedKeyValue>("SerializedKeyValue");

	auto KeyValueArrayType = ToContext->registry->getArray(out.SerializedKeyValue);

	ObjectValue->members.push_back(ClassMember{
		.name = "items",
		.offset = DS_OFFSETOF(ScriptSerializedObject, KeyValueArray),
		.type = KeyValueArrayType,
		});

	IntValue->members.push_back(ClassMember{
		.name = "value",
		.offset = DS_OFFSETOF(ScriptSerializedInt, IntValue),
		.type = IntInst,
		});

	FloatValue->members.push_back(ClassMember{
		.name = "value",
		.offset = DS_OFFSETOF(ScriptSerializedFloat, FloatValue),
		.type = FloatInst,
		});

	out.SerializedKeyValue->members.push_back(ClassMember{
		.name = "name",
		.offset = DS_OFFSETOF(ScriptSerializedKeyValue, NameString),
		.type = StrType,
		});
	out.SerializedKeyValue->members.push_back(ClassMember{
		.name = "value",
		.offset = DS_OFFSETOF(ScriptSerializedKeyValue, Value),
		.type = out.SerializedValue,
		});

	Serialize.addFunction(NativeFunction({FunctionArgument(StrType, "serializedString")},
		ObjectValue->nullable, "parseSerializedString", &Serialize_parseSerializedString));

	Serialize.addFunction(NativeFunction({ FunctionArgument(StrType, "jsonString") },
		out.SerializedValue->nullable, "parseJsonString", &Serialize_parseJsonString));

	ToContext->addNativeModule(Serialize);

	return out;
}

ds::RuntimeClass* engine::script::MarshalSerializedData(SerializedData& DataValue)
{
	auto NameStr = RuntimeStrRef(DataValue.Name.c_str(), DataValue.Name.size());

	auto Value = MarshalSerializedValue(DataValue.Value);

	auto NewObj = NativeModule::makeClass<ScriptSerializedKeyValue>(ScriptSerializedKeyValue{
		.NameString = NameStr.classPtr,
		.Value = Value,
		}, ScriptSerializedKeyValue::ID, &SerializedKeyValue_vTable);

	return NewObj;
}

ds::RuntimeClass* engine::script::MarshalSerializedValue(SerializedValue& Value)
{
	switch (Value.GetType())
	{
	case SerializedData::DataType::Object: {

		std::vector<ds::RuntimeClass*> Items;

		for (auto& i : Value.GetObject())
		{
			Items.push_back(MarshalSerializedData(i));
		}

		auto Array = ds::modules::system::createArray<ds::RuntimeClass*>(Items.data(), Items.size(), true);

		auto NewObj = NativeModule::makeClass<ScriptSerializedObject>({
			Value.GetType(),
			Array,
			}, ScriptSerializedObject::ID, &SerializedObject_vTable);

		return NewObj;
	}
	case SerializedData::DataType::Int32: {

		auto NewObj = NativeModule::makeClass<ScriptSerializedInt>({
			Value.GetType(),
			Value.GetInt(),
			}, ScriptSerializedInt::ID);

		return NewObj;
	}
	case SerializedData::DataType::Float: {

		auto NewObj = NativeModule::makeClass<ScriptSerializedFloat>({
			Value.GetType(),
			Value.GetFloat(),
			}, ScriptSerializedFloat::ID);

		return NewObj;
	}
	}

	return nullptr;
}

#include "SerializeBindings.h"
#include <ds/language.hpp>
#include <ds/modules/system.hpp>
#include <sstream>
#include <Core/File/TextSerializer.h>
#include <Core/File/JsonSerializer.h>
#include <Core/File/BinarySerializer.h>
#include <ds/parser/types/arrayType.hpp>
#include <Core/Log.h>
#include <Engine/File/Resource.h>
#include <Engine/File/AssetRef.h>

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

static void SerializedKeyValue_new(InterpretContext* context)
{
	ClassRef<ScriptSerializedKeyValue> KeyValue = context->popValue<RuntimeClass*>();

	if (!KeyValue.classPtr->vtable)
	{
		KeyValue.classPtr->vtable = &SerializedKeyValue_vTable;
	}

	RuntimeClass* Value = context->popValue<RuntimeClass*>();
	RuntimeClass* Name = context->popValue<RuntimeClass*>();

	KeyValue->Value = Value;
	KeyValue->NameString = Name;

	context->pushValue(KeyValue);
}

static void SerializedInt_new(InterpretContext* context)
{
	ClassRef<ScriptSerializedInt> IntValue = context->popValue<RuntimeClass*>();

	IntValue->Type = SerializedData::DataType::Int32;
	IntValue->IntValue = context->popValue<Int>();

	context->pushValue(IntValue);
}

static void SerializedObject_delete(InterpretContext* context)
{
	auto Value = context->popPtr<ScriptSerializedObject>();

	context->destruct(Value->KeyValueArray);
}

static void SerializedObject_at(InterpretContext* context)
{
	ClassRef<ScriptSerializedObject> Value = context->popValue<RuntimeClass*>();
	RuntimeStr Name = context->popRuntimeString();

	ClassRef<modules::system::ArrayData> Array = Value->KeyValueArray;

	for (Size i = 0; i < Array->length; i++)
	{
		ClassRef<ScriptSerializedKeyValue> Val = Array->at<RuntimeClass*>(i);

		RuntimeStrRef Str = Val->NameString;

		if (strcmp(Str.ptr(), Name.ptr()) == 0)
		{
			Val->Value->addRef();
			context->pushValue(Val->Value);
			return;
		}
	}

	context->pushValue(nullptr);
}

static void SerializedString_delete(InterpretContext* context)
{
	auto Value = context->popPtr<ScriptSerializedString>();

	context->destruct(Value->StringValue);
}

static void SerializedArray_delete(InterpretContext* context)
{
	auto Value = context->popPtr<ScriptSerializedArray>();

	context->destruct(Value->ValueArray);
}

RuntimeFunction SerializedObject_vTable = {
	.nativeFn = &SerializedObject_delete
};

RuntimeFunction SerializedArray_vTable = {
	.nativeFn = &SerializedArray_delete
};

RuntimeFunction SerializedString_vTable = {
	.nativeFn = &SerializedString_delete
};

static void SerializedString_new(InterpretContext* context)
{
	ClassRef<ScriptSerializedString> StringValue = context->popValue<RuntimeClass*>();

	auto str = context->popRuntimeStringRef();

	if (!StringValue.classPtr->vtable)
	{
		StringValue.classPtr->vtable = &SerializedString_vTable;
	}

	StringValue->Type = SerializedData::DataType::String;
	StringValue->StringValue = str.classPtr;

	context->pushValue(StringValue);
}

static void SerializedArray_new(InterpretContext* context)
{
	ClassRef<ScriptSerializedArray> ArrayValue = context->popValue<RuntimeClass*>();

	auto InitialValue = context->popValue<ds::RuntimeClass*>();

	if (!ArrayValue.classPtr->vtable)
	{
		ArrayValue.classPtr->vtable = &SerializedArray_vTable;
	}

	ArrayValue->Type = SerializedData::DataType::Array;
	ArrayValue->ValueArray = InitialValue;

	context->pushValue(ArrayValue);
}

static void SerializedObject_new(InterpretContext* context)
{
	ClassRef<ScriptSerializedObject> ObjectValue = context->popValue<RuntimeClass*>();

	auto InitialValue = context->popValue<ds::RuntimeClass*>();

	if (!ObjectValue.classPtr->vtable)
	{
		ObjectValue.classPtr->vtable = &SerializedObject_vTable;
	}

	ObjectValue->Type = SerializedData::DataType::Object;
	ObjectValue->KeyValueArray = InitialValue;

	context->pushValue(ObjectValue);
}

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

static void Serialize_parseJsonFile(InterpretContext* context)
{
	auto str = context->popRuntimeString();

	std::stringstream Stream;

	Stream << resource::GetTextFile(str.ptr());

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

static void Serialize_parseJsonAsset(InterpretContext* context)
{
	auto ref = context->popPtr<AssetRef*>();

	std::stringstream Stream;

	Stream << resource::GetTextFile((*ref)->FilePath);

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

static void Serialize_parseTextAsset(InterpretContext* context)
{
	auto ref = context->popPtr<AssetRef*>();

	std::stringstream Stream;

	Stream << resource::GetTextFile((*ref)->FilePath);

	try
	{
		SerializedValue Value = TextSerializer::FromStream(Stream);
		context->pushValue(MarshalSerializedValue(Value));
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}

static void Serialize_parseBinaryAsset(InterpretContext* context)
{
	auto fmt = context->popRuntimeString();
	auto ref = context->popPtr<AssetRef*>();
	auto Stream = resource::GetBinaryFile((*ref)->FilePath);

	try
	{
		SerializedValue Value = BinarySerializer::FromStream(Stream, fmt.ptr());
		context->pushValue(MarshalSerializedValue(Value));
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}

static void SerializedValue_writeBinaryFile(InterpretContext* context)
{
	auto Value = context->popValue<RuntimeClass*>();
	auto fmt = context->popRuntimeString();
	auto file = context->popRuntimeString();

	try
	{
		BinarySerializer::ToFile(UnMarshalSerializedValue(Value).GetObject(), file.ptr(), fmt.ptr());
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
	}
}

static void SerializedValue_writeJsonFile(InterpretContext* context)
{
	ClassRef<ScriptSerializedValue> value = context->popValue<RuntimeClass*>();
	auto PrettyPrint = context->popValue<Bool>();
	auto fileName = context->popRuntimeString();

	try
	{
		std::ofstream OutStream = std::ofstream(fileName.ptr());

		OutStream.exceptions(std::ios::failbit | std::ios::badbit);

		try
		{
			JsonSerializer::ToStream(UnMarshalSerializedValue(value.classPtr), OutStream, JsonSerializer::WriteOptions(PrettyPrint));
		}
		catch (SerializeException& e)
		{
			Log::Warn(e.what());
		}
	}
	catch (std::exception& e)
	{
		Log::Warn(e.what());
	}
}

static void SerializedValue_toJsonString(InterpretContext* context)
{
	auto Value = context->popValue<RuntimeClass*>();
	auto PrettyPrint = context->popValue<Bool>();
	try
	{

		std::stringstream Stream;
		JsonSerializer::ToStream(UnMarshalSerializedValue(Value), Stream,
			JsonSerializer::WriteOptions{
				PrettyPrint
			});

		RuntimeStrRef String = RuntimeStrRef(Stream.view().data(), Stream.view().size());
		context->pushValue(String);
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}

static void SerializedValue_toSerializedString(InterpretContext* context)
{
	try
	{
		std::stringstream Stream;
		TextSerializer::ToStream(UnMarshalSerializedValue(context->popValue<RuntimeClass*>()).GetObject(), Stream);

		RuntimeStrRef String = RuntimeStrRef(Stream.view().data(), Stream.view().size());
		context->pushValue(String);
	}
	catch (SerializeException& e)
	{
		Log::Warn(e.what());
		context->pushValue(nullptr);
	}
}

static void SerializedValue_asInt(InterpretContext* context)
{
	auto Value = context->popValue<RuntimeClass*>();
	switch (Value->type)
	{
	case ScriptSerializedInt::ID: {
		ClassRef<ScriptSerializedInt> IntValue = Value;
		context->pushValue(IntValue->IntValue);
		break;
	}
	case ScriptSerializedFloat::ID: {
		ClassRef<ScriptSerializedFloat> FloatValue = Value;
		context->pushValue<Int>(FloatValue->FloatValue);
		break;
	}
	default:
		context->runtimePanic("Failed to convert serialized value to int using asInt()");
		break;
	}
}

static void SerializedValue_asFloat(InterpretContext* context)
{
	auto Value = context->popValue<RuntimeClass*>();
	switch (Value->type)
	{
	case ScriptSerializedInt::ID: {
		ClassRef<ScriptSerializedInt> IntValue = Value;
		context->pushValue<Float>(IntValue->IntValue);
		break;
	}
	case ScriptSerializedFloat::ID: {
		ClassRef<ScriptSerializedFloat> FloatValue = Value;
		context->pushValue(FloatValue->FloatValue);
		break;
	}
	default:
		context->runtimePanic("Failed to convert serialized value to int using asFloat()");
		break;
	}
}

SerializeBindings engine::script::AddSerializeModule(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto IntInst = ToContext->registry->getEntry<IntType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();
	auto AssetRefTypeInst = To.getType("AssetRef");

	SerializeBindings out;

	NativeModule Serialize;

	Serialize.name = "engine::serialize";

	out.SerializedValue = Serialize.createClass<ScriptSerializedValue>("SerializedValue");

	Serialize.addClassMethod(out.SerializedValue, NativeFunction({ FunctionArgument(BoolInst, "prettyPrint") },
		StrType->nullable, "toJsonString", &SerializedValue_toJsonString));

	Serialize.addClassMethod(out.SerializedValue, NativeFunction({},
		StrType->nullable, "toSerializedString", &SerializedValue_toSerializedString));

	Serialize.addClassMethod(out.SerializedValue, NativeFunction({},
		IntInst, "asInt", &SerializedValue_asInt));

	Serialize.addClassMethod(out.SerializedValue, NativeFunction({},
		FloatInst, "asFloat", &SerializedValue_asFloat));

	Serialize.addClassMethod(out.SerializedValue, NativeFunction({ FunctionArgument(StrType, "fileName"), FunctionArgument(StrType, "formatIdentifier") },
		nullptr, "writeBinaryFile", &SerializedValue_writeBinaryFile));

	Serialize.addClassMethod(out.SerializedValue, NativeFunction(
		{ FunctionArgument(StrType, "filePath"), FunctionArgument(BoolInst, "prettyPrint") },
		nullptr, "writeJsonFile", &SerializedValue_writeJsonFile));

	auto ObjectValue = Serialize.createClass<ScriptSerializedObject>("SerializedObject", out.SerializedValue);
	auto ArrayValue = Serialize.createClass<ScriptSerializedArray>("SerializedArray", out.SerializedValue);
	auto IntValue = Serialize.createClass<ScriptSerializedInt>("SerializedInt", out.SerializedValue);
	auto FloatValue = Serialize.createClass<ScriptSerializedFloat>("SerializedFloat", out.SerializedValue);
	auto StringValue = Serialize.createClass<ScriptSerializedString>("SerializedString", out.SerializedValue);
	auto BoolValue = Serialize.createClass<ScriptSerializedString>("SerializedBool", out.SerializedValue);
	out.SerializedKeyValue = Serialize.createClass<ScriptSerializedKeyValue>("SerializedKeyValue");

	auto KeyValueArrayType = ToContext->registry->getArray(out.SerializedKeyValue);
	auto ValueArrayType = ToContext->registry->getArray(out.SerializedValue);

	Serialize.setClassDestructor(out.SerializedKeyValue, NativeFunction(
		{ }, nullptr, "SerializedKeyValue.delete", &SerializedKeyValue_delete));

	Serialize.setClassDestructor(ArrayValue, NativeFunction(
		{ }, nullptr, "SerializedArray.delete", &SerializedArray_delete));

	Serialize.setClassDestructor(ObjectValue, NativeFunction(
		{ }, nullptr, "SerializedObject.delete", &SerializedObject_delete));

	Serialize.addClassConstructor(out.SerializedKeyValue, NativeFunction(
		{ FunctionArgument(StrType, "name"), FunctionArgument(out.SerializedValue, "value") },
		nullptr, "SerializedKeyValue.new", &SerializedKeyValue_new));

	Serialize.addClassConstructor(IntValue, NativeFunction(
		{ FunctionArgument(IntInst, "value") },
		nullptr, "SerializedInt.new", &SerializedInt_new));

	Serialize.addClassConstructor(StringValue, NativeFunction(
		{ FunctionArgument(StrType, "value") },
		nullptr, "SerializedString.new", &SerializedString_new));

	Serialize.addClassConstructor(ArrayValue, NativeFunction(
		{ FunctionArgument(ValueArrayType, "value") },
		nullptr, "SerializedArray.new", &SerializedArray_new));

	Serialize.addClassConstructor(ObjectValue, NativeFunction(
		{ FunctionArgument(KeyValueArrayType, "value") },
		nullptr, "SerializedObject.new", &SerializedObject_new));

	ObjectValue->members.push_back(ClassMember{
		.name = "items",
		.offset = DS_OFFSETOF(ScriptSerializedObject, KeyValueArray),
		.type = KeyValueArrayType,
		});

	ArrayValue->members.push_back(ClassMember{
		.name = "items",
		.offset = DS_OFFSETOF(ScriptSerializedArray, ValueArray),
		.type = ValueArrayType,
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

	StringValue->members.push_back(ClassMember{
		.name = "value",
		.offset = DS_OFFSETOF(ScriptSerializedString, StringValue),
		.type = StrType,
		});

	BoolValue->members.push_back(ClassMember{
		.name = "value",
		.offset = DS_OFFSETOF(ScriptSerializedBool, BoolValue),
		.type = BoolInst,
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

	Serialize.addClassMethod(ObjectValue, NativeFunction({ FunctionArgument(StrType, "name") },
		out.SerializedValue->nullable, "at", &SerializedObject_at));

	Serialize.addFunction(NativeFunction({ FunctionArgument(StrType, "serializedString") },
		ObjectValue->nullable, "parseSerializedString", &Serialize_parseSerializedString));

	Serialize.addFunction(NativeFunction({ FunctionArgument(StrType, "jsonString") },
		out.SerializedValue->nullable, "parseJsonString", &Serialize_parseJsonString));

	Serialize.addFunction(NativeFunction({ FunctionArgument(StrType, "filePath") },
		out.SerializedValue->nullable, "parseJsonFile", &Serialize_parseJsonFile));

	Serialize.addFunction(NativeFunction({ FunctionArgument(AssetRefTypeInst, "asset") },
		out.SerializedValue->nullable, "parseJsonAsset", &Serialize_parseJsonAsset));

	Serialize.addFunction(NativeFunction({ FunctionArgument(AssetRefTypeInst, "asset") },
		ObjectValue->nullable, "parseTextAsset", &Serialize_parseTextAsset));

	Serialize.addFunction(NativeFunction({ FunctionArgument(AssetRefTypeInst, "asset"), FunctionArgument(StrType, "formatIdentifier") },
		out.SerializedValue->nullable, "parseBinaryAsset", &Serialize_parseBinaryAsset));

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
	case SerializedData::DataType::Object:
	{
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
	case SerializedData::DataType::Array:
	{
		std::vector<ds::RuntimeClass*> Items;

		for (auto& i : Value.GetArray())
		{
			Items.push_back(MarshalSerializedValue(i));
		}

		auto Array = ds::modules::system::createArray<ds::RuntimeClass*>(Items.data(), Items.size(), true);

		auto NewObj = NativeModule::makeClass<ScriptSerializedArray>({
			Value.GetType(),
			Array,
			}, ScriptSerializedArray::ID, &SerializedArray_vTable);

		return NewObj;
	}
	case SerializedData::DataType::String:
	{
		auto str = RuntimeStrRef(Value.GetString().c_str(), Value.GetString().size());

		auto NewObj = NativeModule::makeClass<ScriptSerializedString>({
			Value.GetType(),
			str.classPtr,
			}, ScriptSerializedString::ID, &SerializedString_vTable);

		return NewObj;
	}
	case SerializedData::DataType::Int32:
	{
		auto NewObj = NativeModule::makeClass<ScriptSerializedInt>({
			Value.GetType(),
			Value.GetInt(),
			}, ScriptSerializedInt::ID);

		return NewObj;
	}
	case SerializedData::DataType::Float:
	{
		auto NewObj = NativeModule::makeClass<ScriptSerializedFloat>({
			Value.GetType(),
			Value.GetFloat(),
			}, ScriptSerializedFloat::ID);

		return NewObj;
	}
	case SerializedData::DataType::Boolean:
	{
		auto NewObj = NativeModule::makeClass<ScriptSerializedBool>({
			Value.GetType(),
			Value.GetBool(),
			}, ScriptSerializedBool::ID);

		return NewObj;
	}
	case SerializedData::DataType::Vector3:
	{
		auto NewObj = NativeModule::makeClass<ScriptSerializedVector3>({
			Value.GetType(),
			Value.GetVector3(),
			}, ScriptSerializedVector3::ID);

		return NewObj;
	}
	case SerializedData::DataType::Vector2:
	{
		auto NewObj = NativeModule::makeClass<ScriptSerializedVector2>({
			Value.GetType(),
			Value.GetVector2(),
			}, ScriptSerializedVector2::ID);

		return NewObj;
	}
	}

	return nullptr;
}

SerializedData engine::script::UnMarshalSerializedData(ds::RuntimeClass* cls)
{
	ClassRef<ScriptSerializedKeyValue> KeyValue = cls;
	RuntimeStrRef Str = KeyValue->NameString;

	return SerializedData(Str.ptr(), UnMarshalSerializedValue(KeyValue->Value));
}

SerializedValue engine::script::UnMarshalSerializedValue(ds::RuntimeClass* cls)
{
	ClassRef<ScriptSerializedValue> Value = cls;

	switch (Value->Type)
	{
	case SerializedData::DataType::Int32:
	{
		ClassRef<ScriptSerializedInt> IntValue = Value.classPtr;
		return SerializedValue(IntValue->IntValue);
	}
	case SerializedData::DataType::Float:
	{
		ClassRef<ScriptSerializedFloat> FloatValue = Value.classPtr;
		return SerializedValue(FloatValue->FloatValue);
	}
	case SerializedData::DataType::Vector3:
	{
		ClassRef<ScriptSerializedVector3> Vector3Value = Value.classPtr;
		return SerializedValue(Vector3Value->Vector3Value);
	}
	case SerializedData::DataType::Vector2:
	{
		ClassRef<ScriptSerializedVector2> Vector2Value = Value.classPtr;
		return SerializedValue(Vector2Value->Vector2Value);
	}
	case SerializedData::DataType::Object:
	{
		ClassRef<ScriptSerializedObject> ObjectValue = Value.classPtr;

		ClassRef<modules::system::ArrayData> Array = ObjectValue->KeyValueArray;

		std::vector<SerializedData> Object;
		for (Size i = 0; i < Array->length; i++)
		{
			Object.push_back(UnMarshalSerializedData(Array->at<RuntimeClass*>(i)));
		}

		return SerializedValue(Object);
	}
	case SerializedData::DataType::Array:
	{
		ClassRef<ScriptSerializedArray> ObjectValue = Value.classPtr;

		ClassRef<modules::system::ArrayData> Array = ObjectValue->ValueArray;

		std::vector<SerializedValue> ArrayData;
		for (Size i = 0; i < Array->length; i++)
		{
			ArrayData.push_back(UnMarshalSerializedValue(Array->at<RuntimeClass*>(i)));
		}

		return SerializedValue(ArrayData);
	}
	case SerializedData::DataType::String:
	{
		ClassRef<ScriptSerializedString> StringValue = Value.classPtr;
		RuntimeStrRef String = StringValue->StringValue;

		return SerializedValue(String.ptr());
	}
	}

	return SerializedValue();
}

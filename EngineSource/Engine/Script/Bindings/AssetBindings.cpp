#include "AssetBindings.h"
#include <Engine/File/AssetRef.h>
#include <ds/modules/system.hpp>
#include <ds/language.hpp>
#include <ds/parser/types/arrayType.hpp>

using namespace engine;
using namespace ds;
using namespace engine::script;

static void AssetRef_delete(InterpretContext* context)
{
	ClassPtr<AssetRef*> Ref = context->popPtr<AssetRef*>();
	auto ref = *Ref.get();
	delete ref;
}

static RuntimeFunction AssetRef_vTable = RuntimeFunction{
	.nativeFn = &AssetRef_delete
};

static void AssetRef_new(InterpretContext* context)
{
	ClassRef<AssetRef*> NewAssetRef = context->popValue<RuntimeClass*>();
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;
	RuntimeStr FilePath = context->popValue<RuntimeClass*>();

	NewAssetRef.getValue() = new AssetRef(AssetRef::Convert(string(FilePath.ptr(), FilePath.length())));
	context->pushValue(NewAssetRef);
}

static void AssetRef_emptyAsset(InterpretContext* context)
{
	RuntimeStr Extension = context->popRuntimeString();

	ClassRef<AssetRef*> Asset = script::CreateAssetRef();
	Asset.getValue()->Extension = string(Extension.ptr(), Extension.length());
	context->pushValue(Asset);
}

static void ModelData_delete(InterpretContext* context)
{
	ClassPtr<ScriptModelData> Model = context->popPtr<ScriptModelData>();
	context->destruct(Model->Meshes);
	GraphicsModel::UnloadModel(Model->Model);
}

static RuntimeFunction ModelData_vTable = RuntimeFunction{
	.nativeFn = &ModelData_delete
};

static void ModelData_new(InterpretContext* context)
{
	ClassRef<ScriptModelData> Model = context->popValue<RuntimeClass*>();

	static int32 ModelCounter = 0;

	Model.classPtr->vtable = &ModelData_vTable;

	Model->Model = GraphicsModel::RegisterModel(ModelData(), "ScriptModel_" + std::to_string(ModelCounter++));
	Model->Meshes = modules::system::createArray<ModelData::Mesh*>(nullptr, 0, true);

	context->pushValue(Model);
}

static void ModelData_getMeshes(InterpretContext* context)
{
	ClassRef<ScriptModelData> Model = context->popValue<RuntimeClass*>();

	// Copy the array, because any modifications to it do not actually affect the model.
	ClassRef<modules::system::ArrayData> MeshArray = Model->Meshes;

	auto Array = modules::system::createArray<ModelData::Mesh*>(
		reinterpret_cast<ModelData::Mesh**>(MeshArray->data), MeshArray->length, true);

	context->pushValue(Array);
}

ds::RuntimeClass* engine::script::CreateAssetRef()
{
	ClassRef<AssetRef*> NewAssetRef = RuntimeClass::allocateClass(sizeof(AssetRef*), 0, &AssetRef_vTable);
	NewAssetRef.classPtr->vtable = &AssetRef_vTable;

	NewAssetRef.getValue() = new AssetRef();
	return NewAssetRef.classPtr;
}

AssetBindings engine::script::AddAssetBindings(ds::NativeModule& To, ds::LanguageContext* ToContext)
{
	AssetBindings Asset;

	Asset.AssetRef = To.createClass<AssetRef*>("AssetRef");
	auto StrType = ToContext->registry->getEntry<StringType>();
	auto FloatInst = ToContext->registry->getEntry<FloatType>();
	auto BoolInst = ToContext->registry->getEntry<BoolType>();

	auto Vec3Type = To.getType("Vector3");
	auto Vec2Type = To.getType("Vector2");

	To.addClassConstructor(Asset.AssetRef, NativeFunction(
		{ FunctionArgument(StrType, "path") }, nullptr, "AssetRef.new",
		&AssetRef_new));

	To.addFunction(
		NativeFunction({ FunctionArgument(StrType, "extension") },
			Asset.AssetRef, "emptyAsset", &AssetRef_emptyAsset));

	NativeModule AssetModule;
	AssetModule.name = "engine::assets";

	Asset.ModelData = AssetModule.createClass<ScriptModelData>("ModelData");

	auto MeshData = AssetModule.createClass<ModelData::Mesh*>("ModelMesh");

	AssetModule.addClassConstructor(Asset.ModelData, NativeFunction(
		{}, nullptr, "ModelData.new", &ModelData_new));

	auto MeshArray = ToContext->registry->getArray(MeshData);

	AssetModule.addClassMethod(Asset.ModelData, NativeFunction({}, MeshArray, "getMeshes", &ModelData_getMeshes));

	ToContext->addNativeModule(AssetModule);
	return Asset;
}

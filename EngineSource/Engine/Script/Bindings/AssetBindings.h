#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>
#include <Engine/File/ModelData.h>

namespace engine::script
{
	struct AssetBindings
	{
		ds::ClassType* AssetRef = nullptr;
		ds::ClassType* ModelData = nullptr;
	};

	struct ScriptModelData
	{
		GraphicsModel* Model;
		ds::RuntimeClass* Meshes;
	};

	AssetBindings AddAssetBindings(ds::NativeModule& To, ds::LanguageContext* ToContext);

	ds::RuntimeClass* CreateAssetRef();
}
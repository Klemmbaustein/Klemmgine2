#pragma once

namespace engine::script
{
	struct AssetBindings
	{
		ds::NativeStructType* AssetRef = nullptr;
		ds::NativeStructType* ModelData = nullptr;
	};
}
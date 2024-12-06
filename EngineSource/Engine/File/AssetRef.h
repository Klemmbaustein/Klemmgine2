#pragma once
#include <Engine/Types.h>

namespace engine
{
	/**
	* @brief
	* A reference to an Asset file.
	*/
	struct AssetRef
	{
		/**
		* @brief
		* Creates an asset reference from a file name.
		* 
		* Equivalent to writing "FileName.ext"_asset
		*/
		static AssetRef FromName(string Name, string Extension);

		/**
		* @brief
		* Creates an asset reference from a file path.
		*/
		static AssetRef FromPath(string Path);


		/// The path to the referenced file.
		string FilePath;

		string Extension;
	};

}

/**
* @brief
* Creates an asset reference from a file name.
* 
* @see AssetRef::FromName()
*/
engine::AssetRef operator ""_asset(const char*, std::size_t);

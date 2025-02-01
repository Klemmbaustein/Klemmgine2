#pragma once
#include <Core/Types.h>

#if !defined (ENGINE_UTILS_LIB)
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
		[[nodiscard]]
		static AssetRef FromName(string Name, string Extension);

		/**
		* @brief
		* Creates an asset reference from a file path.
		*/
		[[nodiscard]]
		static AssetRef FromPath(string Path);

		[[nodiscard]]
		static AssetRef Convert(std::string PathOrName);

		[[nodiscard]]
		bool IsValid() const;

		/// The path to the referenced file.
		string FilePath;

		string Extension;

		/**
		* @brief
		* Gets the display name of this asset.
		* 
		* The display name is the name of the file (without the path) without the extension.
		* 
		* Example: Assets/Cube.kmdl -> Cube
		*/
		[[nodiscard]]
		string DisplayName() const;

		[[nodiscard]]
		bool Exists() const;
	};

}

/**
* @brief
* Creates an asset reference from a file name.
* 
* @see AssetRef::FromName()
*/
engine::AssetRef operator ""_asset(const char*, std::size_t);
#endif
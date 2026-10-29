#pragma once
#include <Core/Types.h>
#include <optional>

namespace engine
{
	/**
	* @brief
	* A reference to an Asset file.
	*/
	struct AssetRef
	{
		AssetRef() = default;
		AssetRef(const AssetRef&) = default;
		~AssetRef() = default;

		AssetRef& operator=(const AssetRef& Other)
		{
			this->~AssetRef();
			new (this) AssetRef(Other.FilePath, Other.Extension, Other.ExistsCache);
			return *this;
		}

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

		/**
		* @brief
		* Creates an empty asset reference from an extension.
		*/
		[[nodiscard]]
		static AssetRef EmptyAsset(string Extension);

		[[nodiscard]]
		static AssetRef Convert(std::string PathOrName);

		[[nodiscard]]
		bool IsValid() const;

		/// The path to the referenced file.
		const string FilePath;

		/// The extension of the file.
		const string Extension;

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

		bool operator<(const AssetRef& other) const
		{
			return FilePath < other.FilePath;
		}
	private:
		AssetRef(string Path, string Extension, std::optional<bool> ExistsCache);
		AssetRef(string Extension, std::optional<bool> ExistsCache);

		mutable std::optional<bool> ExistsCache = false;
	};

}

/**
* @brief
* Creates an asset reference from a file name.
*
* @see AssetRef::FromName()
*/
engine::AssetRef operator ""_asset(const char*, std::size_t);
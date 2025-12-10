#pragma once
#include <Engine/File/AssetRef.h>
#include <unordered_map>

namespace engine::graphics
{
	struct TextureOptions
	{
		enum Filtering : uint8
		{
			/// Filter to the color value of the nearest value, creating a "pixelated" appearance.
			Nearest,
			/// Filter linearly between the nearest color values, creating a blurry appearance.
			Linear,
		};
		enum BorderType : uint8
		{
			/// Add a black border around the texture for UVs < 0 or > 1
			Border,
			/// Clamp UVs between 0 and 1
			Clamp,
			/// Repeat the texture for UVs < 0 or > 1
			Repeat,
		};

		Filtering Filter = Linear;
		BorderType TextureBorders = Repeat;

		// Use mipmapping on the texture.
		bool MipMaps = true;
	private:
		string Name;
		friend class TextureLoader;
	};

	struct Texture
	{
		TextureOptions Options;
		const uByte* Pixels = nullptr;
		uint32 TextureObject = 0;
		mutable uint32 References = 0;
		uint32 Width = 0, Height = 0;
	};


	class TextureLoader
	{
	public:
		TextureLoader();
		~TextureLoader();

		[[nodiscard]]
		const Texture* LoadTextureFile(AssetRef From, TextureOptions LoadInfo);

		[[nodiscard]]
		const Texture* LoadCompressedBuffer(const uByte* Buffer, size_t BufferSize, TextureOptions LoadInfo);

		/**
		 * @brief
		 * Loads a texture from the given RGBA pixel buffer.
		 * @param Pixels
		 * The pixel buffer in the RGBA8888 format.
		 * @param Width
		 * The width of the texture in the buffer
		 * @param Height
		 * The Height of the texture in the buffer.
		 * @param LoadInfo
		 * Load options for the texture, defining texture filtering, clamping, mipmaps etc.
		 * @return
		 * A pointer to the new texture object that should be freed using FreeTexture() once it is no longer needed.
		 */
		[[nodiscard]]
		const Texture* LoadTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo);

		const Texture* PreLoadBuffer(AssetRef From, TextureOptions LoadInfo);

		void FreeTexture(const Texture* Tex);
		void FreeTexture(AssetRef From);

		/**
		 * @brief
		 * The current texture loader instance.
		 */
		static TextureLoader* Instance;

	private:

		static void FreeTextureData(const Texture* Tex);

		static string MakeTextureID(const AssetRef& Ref, const TextureOptions& LoadInfo);

		uint32 CreateGLTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo);
		void DeleteTexture(const Texture* Tex);
		std::unordered_map<string, Texture> LoadedTextures;
	};
}
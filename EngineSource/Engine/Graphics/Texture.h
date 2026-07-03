#pragma once
#include <Engine/File/AssetRef.h>
#include <Engine/Graphics/Backend/Renderer.h>
#include <unordered_map>

namespace engine::graphics
{
	struct Texture
	{
		TextureOptions Options;
		const uByte* Pixels = nullptr;
		RendererTexture* RenderTexture = nullptr;
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

		Renderer* UsedRenderer = nullptr;

	private:

		static void FreeTextureData(const Texture* Tex);

		static string MakeTextureID(const AssetRef& Ref, const TextureOptions& LoadInfo);

		RendererTexture* CreateRendererTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo);
		void DeleteTexture(const Texture* Tex);
		std::unordered_map<string, Texture> LoadedTextures;
	};
}
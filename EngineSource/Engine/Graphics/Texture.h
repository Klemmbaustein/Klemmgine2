#pragma once
#include <Engine/File/AssetRef.h>
#include <unordered_map>

namespace engine::graphics
{
	struct TextureOptions
	{
		enum Filtering : uint8
		{
			Nearest,
			Linear,
		};
		enum BorderType : uint8
		{
			Border,
			Clamp,
			Repeat,
		};

		Filtering Filter = Linear;
		BorderType TextureBorders = Repeat;

		bool MipMaps = false;
	private:
		string Name;
		friend class TextureLoader;
	};

	struct Texture
	{
		TextureOptions Options;
		const uByte* Pixels = nullptr;
		uint32 TextureObject = 0;
		uint32 References = 0;
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

		[[nodiscard]]
		const Texture* LoadTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo);

		const Texture* PreLoadBuffer(AssetRef From, TextureOptions LoadInfo);

		void FreeTexture(const Texture* Tex);

		static TextureLoader* Instance;

	private:
		uint32 CreateGLTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo);
		void DeleteTexture(const Texture* Tex);
		std::unordered_map<string, Texture> LoadedTextures;
		std::unordered_map<string, Texture> TextureBuffers;
	};
}
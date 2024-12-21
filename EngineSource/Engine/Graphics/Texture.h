#pragma once
#include <Engine/File/AssetRef.h>
#include <unordered_map>

namespace engine::graphics
{
	struct TextureLoadOptions
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

		Filtering Filter = Nearest;
		BorderType TextureBorders = Repeat;

		bool MipMaps = false;
	private:
		string Name;
		friend class TextureLoader;
	};

	struct Texture
	{
		TextureLoadOptions Options;
		const uByte* Pixels = nullptr;
		uint32 TextureObject = 0;
		uint32 References = 0;
	};


	class TextureLoader
	{
	public:
		TextureLoader();
		~TextureLoader();

		[[nodiscard]]
		const Texture* LoadTextureFile(AssetRef From, TextureLoadOptions LoadInfo);

		[[nodiscard]]
		const Texture* LoadCompressedBuffer(const uByte* Buffer, size_t BufferSize, TextureLoadOptions LoadInfo);

		[[nodiscard]]
		const Texture* LoadTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureLoadOptions LoadInfo);

		void FreeTexture(const Texture* Tex);

		static TextureLoader* Instance;

	private:
		uint32 CreateGLTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureLoadOptions LoadInfo);
		void DeleteTexture(const Texture* Tex);
		static std::unordered_map<string, Texture> LoadedTextures;
	};
}
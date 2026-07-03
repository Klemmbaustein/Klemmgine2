#include "Texture.h"
#include <Engine/File/Resource.h>
#include <mutex>
#include <stb_image.hpp>
using namespace engine;
using namespace engine::graphics;

TextureLoader* TextureLoader::Instance = nullptr;
std::mutex TextureLoadMutex;

graphics::TextureLoader::TextureLoader()
{
	Instance = this;
}

graphics::TextureLoader::~TextureLoader()
{
}

const Texture* TextureLoader::LoadTextureFile(AssetRef From, TextureOptions LoadInfo)
{
	using namespace engine::resource;

	auto Name = MakeTextureID(From, LoadInfo);

	{
		TextureLoadMutex.lock();

		// Check if the texture is fully loaded already.
		if (LoadedTextures.contains(Name))
		{
			Texture& tx = LoadedTextures[Name];

			if (tx.Pixels && !tx.RenderTexture)
			{
				tx.RenderTexture = CreateRendererTexture(tx.Pixels, tx.Width, tx.Height, tx.Options);
				stbi_image_free(const_cast<uByte*>(tx.Pixels));
				tx.Pixels = nullptr;
			}

			tx.References++;
			TextureLoadMutex.unlock();
			return &tx;
		}

		LoadInfo.Name = Name;
		TextureLoadMutex.unlock();
	}

	IBinaryStream* Bytes = GetBinaryFile(From.FilePath);
	if (!Bytes)
		return nullptr;

	string str;
	Bytes->ReadAll(str);

	const Texture* Result = LoadCompressedBuffer((stbi_uc*)str.data(), Bytes->GetSize(), LoadInfo);
	delete Bytes;

	return Result;
}

const Texture* TextureLoader::LoadCompressedBuffer(const uByte* Buffer, size_t BufferSize, TextureOptions LoadInfo)
{
	int w, h, ch;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* Loaded = stbi_load_from_memory(Buffer, int(BufferSize), &w, &h, &ch, 4);
	const Texture* Out = LoadTexture(Loaded, w, h, LoadInfo);
	stbi_image_free(Loaded);
	return Out;
}

const Texture* TextureLoader::LoadTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo)
{
	RendererTexture* TextureID = CreateRendererTexture(Pixels, Width, Height, LoadInfo);
	{
		std::lock_guard g{ TextureLoadMutex };

		return &(*LoadedTextures.insert({ LoadInfo.Name, Texture{
			.Options = LoadInfo,
			.Pixels = TextureID ? nullptr : Pixels,
			.RenderTexture = TextureID,
			.References = 1,
			} }).first).second;
	}
}

const Texture* engine::graphics::TextureLoader::PreLoadBuffer(AssetRef From, TextureOptions LoadInfo)
{
	using namespace engine::resource;

	std::lock_guard g{ TextureLoadMutex };

	auto Name = MakeTextureID(From, LoadInfo);

	if (LoadedTextures.contains(Name))
	{
		auto& tx = LoadedTextures[Name];
		tx.References++;
		return &tx;
	}

	LoadInfo.Name = Name;

	int w, h, ch;
	IBinaryStream* Bytes = GetBinaryFile(From.FilePath);

	if (!Bytes)
	{
		return nullptr;
	}

	stbi_set_flip_vertically_on_load(true);

	string str;
	Bytes->ReadAll(str);

	auto Pixels = stbi_load_from_memory((stbi_uc*)str.data(), int(Bytes->GetSize()), &w, &h, &ch, 4);
	const Texture* New = &(*LoadedTextures.insert({ Name, Texture{
		.Options = LoadInfo,
		.Pixels = Pixels,
		.RenderTexture = nullptr,
		.References = 1,
		.Width = uint32(w),
		.Height = uint32(h),
	} }).first).second;

	delete Bytes;
	return New;
}

void TextureLoader::FreeTexture(const Texture* Tex)
{
	for (auto& [id, texture] : LoadedTextures)
	{
		if (&texture == Tex)
		{
			texture.References--;
			if (texture.References == 0)
			{
				FreeTextureData(&texture);
				LoadedTextures.erase(id);
			}
			return;
		}
	}

	Tex->References--;
	if (Tex->References == 0)
	{
		FreeTextureData(Tex);
	}
}

void engine::graphics::TextureLoader::FreeTexture(AssetRef From)
{
}

void engine::graphics::TextureLoader::FreeTextureData(const Texture* Tex)
{
	if (Tex->Pixels)
	{
		stbi_image_free(const_cast<uByte*>(Tex->Pixels));
	}

	if (Tex->RenderTexture)
	{
		delete Tex->RenderTexture;
	}
}

string engine::graphics::TextureLoader::MakeTextureID(const AssetRef& Ref, const TextureOptions& LoadInfo)
{
	return Ref.FilePath + std::to_string(LoadInfo.Filter) + std::to_string(LoadInfo.TextureBorders);
}

RendererTexture* engine::graphics::TextureLoader::CreateRendererTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureOptions LoadInfo)
{
	return this->UsedRenderer->CreateTexture(Pixels, Width, Height, LoadInfo);
}

void engine::graphics::TextureLoader::DeleteTexture(const Texture* Tex)
{
}

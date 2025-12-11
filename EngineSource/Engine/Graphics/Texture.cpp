#include "Texture.h"
#include <Core/Log.h>
#include <Engine/File/Resource.h>
#include <Engine/Internal/OpenGL.h>
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

			if (tx.Pixels && !tx.TextureObject)
			{
				tx.TextureObject = CreateGLTexture(tx.Pixels, tx.Width, tx.Height, tx.Options);
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

	ReadOnlyBufferStream* Bytes = GetBinaryFile(From.FilePath);
	if (!Bytes)
		return nullptr;
	const Texture* Result = LoadCompressedBuffer(Bytes->GetData(), Bytes->GetSize(), LoadInfo);
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
	uint32 TextureID = CreateGLTexture(Pixels, Width, Height, LoadInfo);
	{
		std::lock_guard g{ TextureLoadMutex };

		return &(*LoadedTextures.insert({ LoadInfo.Name, Texture{
			.Options = LoadInfo,
			.Pixels = TextureID ? nullptr : Pixels,
			.TextureObject = TextureID,
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
	ReadOnlyBufferStream* Bytes = GetBinaryFile(From.FilePath);

	if (!Bytes)
	{
		return nullptr;
	}

	stbi_set_flip_vertically_on_load(true);
	auto Pixels = stbi_load_from_memory(Bytes->GetData(), int(Bytes->GetSize()), &w, &h, &ch, 4);
	const Texture* New = &(*LoadedTextures.insert({ Name, Texture{
		.Options = LoadInfo,
		.Pixels = Pixels,
		.TextureObject = 0,
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

	if (Tex->TextureObject)
	{
		glDeleteTextures(1, &Tex->TextureObject);
	}
}

string engine::graphics::TextureLoader::MakeTextureID(const AssetRef& Ref, const TextureOptions& LoadInfo)
{
	return Ref.FilePath + std::to_string(LoadInfo.Filter) + std::to_string(LoadInfo.TextureBorders);
}

uint32 engine::graphics::TextureLoader::CreateGLTexture(const uByte* Pixels, uint64 Width, uint64 Height,
	TextureOptions LoadInfo)
{
	uint32 TextureID = 0;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);


	GLint WrapMode = GL_CLAMP_TO_BORDER;

	switch (LoadInfo.TextureBorders)
	{
	case TextureOptions::Border:
		WrapMode = GL_CLAMP_TO_BORDER;
		break;
	case TextureOptions::Repeat:
		WrapMode = GL_REPEAT;
		break;
	case TextureOptions::Clamp:
		WrapMode = GL_CLAMP_TO_EDGE;
		break;
	default:
		break;
	}
	GLint Filter = LoadInfo.Filter == TextureOptions::Linear ? GL_LINEAR : GL_NEAREST;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LoadInfo.MipMaps ? GL_LINEAR_MIPMAP_LINEAR : Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)Width, (GLsizei)Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	return TextureID;
}

void engine::graphics::TextureLoader::DeleteTexture(const Texture* Tex)
{
}

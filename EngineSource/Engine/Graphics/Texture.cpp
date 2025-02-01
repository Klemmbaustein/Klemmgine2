#include "Texture.h"
#include <Engine/File/Resource.h>
#include <Core/Error/EngineAssert.h>
#include <Engine/Internal/OpenGL.h>
#include <Engine/MainThread.h>
#include <stb_image.hpp>
#include <mutex>
using namespace engine;
using namespace engine::graphics;

std::unordered_map<string, Texture> TextureLoader::LoadedTextures;
TextureLoader* TextureLoader::Instance = nullptr;
std::mutex TextureLoadMutex;

graphics::TextureLoader::TextureLoader()
{
	Instance = this;
}

graphics::TextureLoader::~TextureLoader()
{
}

const Texture* TextureLoader::LoadTextureFile(AssetRef From, TextureLoadOptions LoadInfo)
{
	using namespace engine::resource;

	{
		std::lock_guard g{ TextureLoadMutex };
		if (LoadedTextures.contains(From.FilePath))
		{
			Texture& tx = LoadedTextures[From.FilePath];

			tx.References++;
			return &tx;
		}
	}
	LoadInfo.Name = From.FilePath;

	BinaryFile Bytes = GetBinaryFile(From.FilePath);
	if (!Bytes.DataPtr)
		return nullptr;
	const Texture* Result = LoadCompressedBuffer(Bytes.DataPtr, Bytes.DataSize, LoadInfo);
	if (!Result->Pixels)
	{
		FreeBinaryFile(Bytes);
	}

	return Result;
}

const Texture* TextureLoader::LoadCompressedBuffer(const uByte* Buffer, size_t BufferSize, TextureLoadOptions LoadInfo)
{
	int w, h, ch;
	stbi_uc* Loaded = stbi_load_from_memory(Buffer, int(BufferSize), &w, &h, &ch, 4);
	const Texture* Out = LoadTexture(Loaded, w, h, LoadInfo);
	stbi_image_free(Loaded);
	return Out;
}

const Texture* TextureLoader::LoadTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureLoadOptions LoadInfo)
{
	uint32 TextureID = 0;
	if (thread::IsMainThread)
	{
		TextureID = CreateGLTexture(Pixels, Width, Height, LoadInfo);
	}
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

void TextureLoader::FreeTexture(const Texture* Tex)
{
}

uint32 engine::graphics::TextureLoader::CreateGLTexture(const uByte* Pixels, uint64 Width, uint64 Height, TextureLoadOptions LoadInfo)
{
	uint32 TextureID = 0;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)Width, (GLsizei)Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
	return TextureID;
}

void engine::graphics::TextureLoader::DeleteTexture(const Texture* Tex)
{
}

#pragma once
#include <kui/Window.h>
#include <Core/Types.h>
#include <Engine/Graphics/Vertex.h>
#include <Core/Transform.h>

namespace engine::graphics
{
	class ShaderObject;

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
		bool MipMaps = false;
		bool IsFloatTexture = false;
		bool HasAlphaChannel = true;
	private:
		string Name;
		friend class TextureLoader;
	};

	class RendererTexture
	{
	public:
		virtual ~RendererTexture() = default;

		virtual uint32 GetUITexture() = 0;
		virtual void SetFilterMode(TextureOptions::Filtering NewFilter, size_t TextureIndex) = 0;
	};

	enum class DrawTargetBufferType
	{
		Color,
		Depth,
		DepthStencil,
	};

	class DrawTargetBuffer
	{
	public:
		DrawTargetBufferType Type = DrawTargetBufferType::Color;
		bool FilterNearest = true;
	};

	class RendererDrawTarget
	{
	public:
		virtual ~RendererDrawTarget() = default;
		virtual RendererTexture* GetTexture(size_t Index) = 0;
		virtual RendererTexture* GetStencilTexture() = 0;

		virtual void Activate() = 0;
		virtual void Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil) = 0;
	};

	class DrawUniformBuffer
	{
	public:
		virtual ~DrawUniformBuffer() = default;

		virtual void Write(size_t Offset, void* Data, size_t DataSize) = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Draw() = 0;
	};

	class DrawCommand
	{
	public:
		virtual ~DrawCommand() = default;

		virtual void SetResolution(uint32 Width, uint32 Height) = 0;
		virtual void SetFaceCullEnabled(bool NewEnabled) = 0;
		virtual void SetDepthCheckEnabled(bool NewEnabled) = 0;
		virtual void SetBlendEnabled(bool NewEnabled) = 0;
		virtual void SetStencilValue(bool Enabled, uint8 Value) = 0;

		virtual void UseShader(ShaderObject* TargetShader) = 0;

		virtual void BindTexture(const string& UniformName, RendererTexture* Target) = 0;
		virtual void BindTexture(uint32 UniformLocation, RendererTexture* Target) = 0;

		virtual void BindUniformBlock(const string& UniformBlockName, DrawUniformBuffer* Target) = 0;

		virtual void DrawVertices(size_t Count) = 0;
		virtual void DrawVertexBuffer(VertexBuffer* Buffer) = 0;
		virtual void ResetTextures() = 0;
	};

	enum class ShaderProgramType
	{
		Vertex,
		Fragment,
		Geometry,
	};

	class ShaderProgramObject
	{
	public:
		virtual ~ShaderProgramObject() = default;
	};

	class ShaderProgram
	{
	public:
		virtual ~ShaderProgram() = default;

		virtual void Activate() = 0;
		virtual uint32 GetUniformLocation(const char* Name) = 0;
		virtual uint32 GetUniformBlockLocation(const char* Name) = 0;

		virtual void SetInt(uint32 UniformLocation, int32 Value) = 0;
		virtual void SetFloat(uint32 UniformLocation, float Value) = 0;
		virtual void SetVec3(uint32 UniformLocation, Vector3 Value) = 0;
		virtual void SetVec2(uint32 UniformLocation, Vector2 Value) = 0;
		virtual void SetMatrix(uint32 UniformLocation, const glm::mat4& Value) = 0;
	};

	class Renderer
	{
	public:

		virtual ~Renderer() = default;

		virtual void RenderScreen(kui::Window* WithWindow, RendererTexture* Texture, bool VSync) = 0;
		virtual RendererTexture* CreateTexture(const uByte* Pixels, uint32 Width, uint32 Height,
			const TextureOptions& Options) = 0;
		virtual RendererDrawTarget* CreateDrawTarget(uint32 Width, uint32 Height, std::vector<DrawTargetBuffer> Buffers) = 0;
		virtual RendererDrawTarget* CreateShadowMaps(uint32 Width, uint32 Height, uint32 Count) = 0;
		virtual VertexBuffer* CreateVertexBuffer(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices) = 0;
		virtual DrawCommand* StartRender() = 0;
		virtual DrawUniformBuffer* CreateUniformBuffer(size_t Size) = 0;
		virtual ShaderProgramObject* CreateShaderProgramObject(const string& Source, ShaderProgramType Type) = 0;
		virtual ShaderProgram* LinkShaderProgram(std::vector<ShaderProgramObject*> Objects) = 0;
	};
}
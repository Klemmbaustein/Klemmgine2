#pragma once
#include <Engine/Graphics/Backend/Renderer.h>
#include <utility>

namespace engine::graphics
{
	class OpenGLRenderer;

	class OpenGLRendererTexture : public RendererTexture
	{
	public:
		OpenGLRendererTexture(OpenGLRenderer* Render)
			: Render(Render)
		{
		}
		~OpenGLRendererTexture() = default;

		uint32 GetUITexture() override;
		void SetFilterMode(TextureOptions::Filtering NewFilter, size_t TextureIndex) override;

		uint32 TypeEnum = 0;
		uint32 TextureObject = 0;

	protected:
		OpenGLRenderer* Render = nullptr;
	};

	class OpenGLImageRendererTexture : public OpenGLRendererTexture
	{
	public:
		OpenGLImageRendererTexture(const uByte* Pixels, uint32 Width, uint32 Height,
			const TextureOptions& Options, OpenGLRenderer* Render);
		~OpenGLImageRendererTexture() override;
	};

	class OpenGLRendererDrawTarget : public RendererDrawTarget
	{
	public:
		uint32 Buffer = 0;

		OpenGLRendererDrawTarget(uint32 Width, uint32 Height, std::vector<DrawTargetBuffer> Buffers, OpenGLRenderer* Render);
		~OpenGLRendererDrawTarget();

		class TargetTexture : public OpenGLRendererTexture
		{
		public:
			TargetTexture(uint32 t, OpenGLRenderer* Render);

			bool IsDepthStencil = false;
			bool IsStencilMode = false;

			void SetStencilMode(bool NewIsStencilMode);

			uint32 GetUITexture() override;
			void SetFilterMode(TextureOptions::Filtering NewFilter, size_t TextureIndex) override;
		};

		std::vector<TargetTexture> Textures;
		size_t StencilIndex = SIZE_MAX;

		// Inherited via RendererDrawTarget
		void Activate() override;
		void Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil) override;
		RendererTexture* GetTexture(size_t Index) override;
		RendererTexture* GetStencilTexture() override;

	private:
		uint32 Width = 0;
		uint32 Height = 0;
		OpenGLRenderer* Render = nullptr;
	};

	class OpenGLDrawUniformBuffer : public DrawUniformBuffer
	{
	public:

		OpenGLDrawUniformBuffer(size_t Size, OpenGLRenderer* Render);

		// Inherited via DrawUniformBuffer
		void Write(size_t Offset, void* Data, size_t DataSize) override;

		uint32 BufferObject = 0;
		uint32 BufferId = 0;

	protected:
		OpenGLRenderer* Render = nullptr;
	};

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const std::vector<Vertex>& Vertices, const std::vector<uint32>& Indices);
		~OpenGLVertexBuffer() override;

		// Inherited via VertexBuffer
		void Draw() override;

		uint32 VAO = 0u, VBO = 0u, EBO = 0u, IndicesSize = 0u;
	};

	class OpenGLDrawCommand : public DrawCommand
	{
	public:
		OpenGLDrawCommand(OpenGLRenderer* Render);

		// Inherited via DrawCommand
		void UseShader(ShaderObject* TargetShader) override;
		void BindTexture(const string& UniformName, RendererTexture* Target) override;
		void BindTexture(uint32 UniformLocation, RendererTexture* Target) override;
		void DrawVertices(size_t Count) override;
		void DrawVertexBuffer(VertexBuffer* Buffer) override;
		void ResetTextures() override;
		void SetResolution(uint32 Width, uint32 Height) override;
		void SetFaceCullEnabled(bool NewEnabled) override;
		void SetDepthCheckEnabled(bool NewEnabled) override;
		void SetBlendEnabled(bool NewEnabled) override;
		void SetStencilValue(bool Enabled, uint8 Value) override;
		void BindUniformBlock(const string& UniformBlockName, DrawUniformBuffer* Target) override;

		void Reset();

	private:
		void Apply();

		size_t TextureSlotCounter = 1;

		uint32 ViewportWidth = 0;
		uint32 ViewportHeight = 0;

		uint8 StencilValue = 0;

		bool DepthTestEnabled = false;
		bool BlendEnabled = false;
		bool FaceCullEnabled = false;
		bool DepthCheckEnabled = true;
		bool StencilEnabled = false;

		OpenGLRenderer* Render = nullptr;
		ShaderObject* CurrentShader = nullptr;
	};

	class RendererShadowDrawTarget : public RendererDrawTarget, public OpenGLRendererTexture
	{
	public:

		RendererShadowDrawTarget(uint32 Width, uint32 Height, uint32 Count, OpenGLRenderer* Render);

		// Inherited via RendererDrawTarget
		RendererTexture* GetTexture(size_t Index) override;
		RendererTexture* GetStencilTexture() override;
		void Activate() override;

		uint32 LightFBO = 0;

	private:
		uint32 Width = 0;
		uint32 Height = 0;
		OpenGLRenderer* Render = nullptr;

		// Inherited via RendererDrawTarget
		void Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil) override;
	};

	struct TextureSlot
	{
		uint32 Item = 0;
	};

	class OpenGLShaderProgramObject : public ShaderProgramObject
	{
	public:
		OpenGLShaderProgramObject(const string& Source, ShaderProgramType Type);
		~OpenGLShaderProgramObject();

		uint32 CompiledObject = 0;
	};

	class OpenGLShaderProgram : public ShaderProgram
	{
	public:
		static bool CheckCompileErrors(uint32 ShaderID, string Type);

		OpenGLShaderProgram(std::vector<ShaderProgramObject*> Objects, OpenGLRenderer* Render);
		~OpenGLShaderProgram();

		uint32 ProgramObject = 0;

		// Inherited via ShaderProgram
		void Activate() override;
		uint32 GetUniformLocation(const char* Name) override;
		uint32 GetUniformBlockLocation(const char* Name) override;
		void SetInt(uint32 UniformLocation, int32 Value) override;
		void SetFloat(uint32 UniformLocation, float Value) override;
		void SetVec3(uint32 UniformLocation, Vector3 Value) override;
		void SetVec2(uint32 UniformLocation, Vector2 Value) override;
		void SetMatrix(uint32 UniformLocation, const glm::mat4& Value) override;

	private:
		OpenGLRenderer* Render = nullptr;
	};

	class ShaderCompilerError
	{

	};

	class OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer();

		// Inherited via Renderer
		void RenderScreen(kui::Window* WithWindow, RendererTexture* Texture, bool VSync) override;
		OpenGLRendererTexture* CreateTexture(const uByte* Pixels, uint32 Width, uint32 Height,
			const TextureOptions& Options) override;
		OpenGLRendererDrawTarget* CreateDrawTarget(uint32 Width, uint32 Height,
			std::vector<DrawTargetBuffer> Buffers) override;
		RendererShadowDrawTarget* CreateShadowMaps(uint32 Width, uint32 Height, uint32 Count) override;
		OpenGLVertexBuffer* CreateVertexBuffer(const std::vector<Vertex>& Vertices,
			const std::vector<uint32>& Indices) override;
		OpenGLDrawCommand* StartRender() override
		{
			CurrentCommand = OpenGLDrawCommand(this);
			return &CurrentCommand;
		}

		DrawUniformBuffer* CreateUniformBuffer(size_t Size) override;
		ShaderProgramObject* CreateShaderProgramObject(const string& Source, ShaderProgramType Type) override;
		ShaderProgram* LinkShaderProgram(std::vector<ShaderProgramObject*> Objects) override;

		OpenGLDrawCommand CurrentCommand = OpenGLDrawCommand(this);

		void ActivateFramebuffer(uint32 BufferObject);
		void ActivateTexture(uint32 TextureObject, uint8 Slot);

		void ActivateTexture(uint32 TextureObject, uint8 Slot, uint32 Type);

		void SetRenderResolution(uint32 NewWidth, uint32 NewHeight);
		void SetFaceCullEnabled(bool NewEnabled);
		void SetDepthCheckEnabled(bool NewEnabled);
		void SetBlendEnabled(bool NewEnabled);
		void SetStencilEnabled(bool NewEnabled, uint8 Mask);
		void SetStencilValue(uint8 NewValue, uint8 Mask);
		void SetStencilMask(uint8 Mask);

		void UseProgram(uint32 NewProgram);

		uint32 ViewportWidth = 0;
		uint32 ViewportHeight = 0;

		uint32 FramebufferWidth = 0;
		uint32 FramebufferHeight = 0;

		uint8 StencilTestValue = 0;
		uint8 ActiveTextureSlot = 0;
		uint32 ActiveTextureObject = 0;
		uint32 ActiveFramebuffer = UINT32_MAX;
		uint32 UniformBufferIndex = 0;
		uint32 CurrentShaderProgram = UINT32_MAX;

		bool DepthTestEnabled = false;
		bool BlendEnabled = false;
		bool StencilWriteEnabled = false;
		bool FaceCullEnabled = false;
		bool VSyncEnabled = true;
	};
}
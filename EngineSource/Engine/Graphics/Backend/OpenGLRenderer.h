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

			uint32 GetUITexture() final override;
			void SetFilterMode(TextureOptions::Filtering NewFilter, size_t TextureIndex) final override;
		};

		std::vector<TargetTexture> Textures;
		size_t StencilIndex = SIZE_MAX;

		// Inherited via RendererDrawTarget
		void Activate() override;
		void Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil) final override;
		RendererTexture* GetTexture(size_t Index) final override;
		RendererTexture* GetStencilTexture() final override;

	private:
		uint32 Width = 0;
		uint32 Height = 0;
		OpenGLRenderer* Render = nullptr;
	};

	class OpenGLDrawUniformBuffer : public DrawUniformBuffer
	{
	public:

		OpenGLDrawUniformBuffer(size_t Size, OpenGLRenderer* Render);
		~OpenGLDrawUniformBuffer();

		// Inherited via DrawUniformBuffer
		void Write(size_t Offset, void* Data, size_t DataSize) final override;

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
		void UseShader(ShaderObject* TargetShader) final override;
		void BindTexture(const string& UniformName, RendererTexture* Target) final override;
		void BindTexture(uint32 UniformLocation, RendererTexture* Target) final override;
		void DrawVertices(size_t Count) final override;
		void DrawVertexBuffer(VertexBuffer* Buffer) final override;
		void ResetTextures() final override;
		void SetResolution(uint32 Width, uint32 Height) final override;
		void SetFaceCullEnabled(bool NewEnabled) final override;
		void SetDepthCheckEnabled(bool NewEnabled) final override;
		void SetBlendEnabled(bool NewEnabled) final override;
		void SetStencilValue(bool Enabled, uint8 Value) final override;
		void BindUniformBlock(const string& UniformBlockName, DrawUniformBuffer* Target) final override;

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
		~RendererShadowDrawTarget();

		// Inherited via RendererDrawTarget
		RendererTexture* GetTexture(size_t Index) final override;
		RendererTexture* GetStencilTexture() final override;
		void Activate() final override;
		void Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil) final override;

		uint32 LightFBO = 0;

	private:
		uint32 Width = 0;
		uint32 Height = 0;
		OpenGLRenderer* Render = nullptr;
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
		void Activate() final override;
		uint32 GetUniformLocation(const char* Name) final override;
		uint32 GetUniformBlockLocation(const char* Name) final override;
		void SetInt(uint32 UniformLocation, int32 Value) final override;
		void SetFloat(uint32 UniformLocation, float Value) final override;
		void SetVec3(uint32 UniformLocation, Vector3 Value) final override;
		void SetVec2(uint32 UniformLocation, Vector2 Value) final override;
		void SetMatrix(uint32 UniformLocation, const glm::mat4& Value) final override;

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
		void RenderScreen(kui::Window* WithWindow, RendererTexture* Texture, bool VSync) final override;
		OpenGLRendererTexture* CreateTexture(const uByte* Pixels, uint32 Width, uint32 Height,
			const TextureOptions& Options) final override;
		OpenGLRendererDrawTarget* CreateDrawTarget(uint32 Width, uint32 Height,
			std::vector<DrawTargetBuffer> Buffers) final override;
		RendererShadowDrawTarget* CreateShadowMaps(uint32 Width, uint32 Height, uint32 Count) final override;
		OpenGLVertexBuffer* CreateVertexBuffer(const std::vector<Vertex>& Vertices,
			const std::vector<uint32>& Indices) final override;
		OpenGLDrawCommand* StartRender() final override
		{
			CurrentCommand = OpenGLDrawCommand(this);
			return &CurrentCommand;
		}

		DrawUniformBuffer* CreateUniformBuffer(size_t Size) final override;
		ShaderProgramObject* CreateShaderProgramObject(const string& Source, ShaderProgramType Type) final override;
		ShaderProgram* LinkShaderProgram(std::vector<ShaderProgramObject*> Objects) final override;
		bool SupportsUniformBuffer() final override;

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
		bool VSyncEnabled = false;
	};
}
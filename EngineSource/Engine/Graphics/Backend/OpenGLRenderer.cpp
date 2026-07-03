#include "OpenGLRenderer.h"
#include <Engine/Graphics/Backend/OpenGL.h>
#include <kui/Rendering/OpenGLBackend.h>
#include <SDL3/SDL.h>
#include <Engine/Internal/SystemWM_SDL3.h>
#include <Engine/Graphics/ShaderObject.h>
#include <Engine/Graphics/VideoSubsystem.h>

#ifdef EDITOR
#include <Editor/EditorSubsystem.h>
#include <Editor/UI/Panels/Viewport.h>
#endif

static kui::systemWM::SysWindow* GetSysWindow(kui::Window* From)
{
	return static_cast<kui::systemWM::SysWindow*>(From->GetSysWindow());
}

using namespace engine;
using namespace engine::graphics;
using namespace kui;

OpenGLRendererTexture* engine::graphics::OpenGLRenderer::CreateTexture(const uByte* Pixels, uint32 Width,
	uint32 Height, const TextureOptions& Options)
{
	return new OpenGLImageRendererTexture(Pixels, Width, Height, Options, this);
}

OpenGLRendererDrawTarget* engine::graphics::OpenGLRenderer::CreateDrawTarget(uint32 Width, uint32 Height,
	std::vector<DrawTargetBuffer> Buffers)
{
	return new OpenGLRendererDrawTarget(Width, Height, Buffers, this);
}

RendererShadowDrawTarget* engine::graphics::OpenGLRenderer::CreateShadowMaps(uint32 Width, uint32 Height, uint32 Count)
{
	return new RendererShadowDrawTarget(Width, Height, Count, this);
}

OpenGLVertexBuffer* engine::graphics::OpenGLRenderer::CreateVertexBuffer(const std::vector<Vertex>& Vertices,
	const std::vector<uint32>& Indices)
{
	return new OpenGLVertexBuffer(Vertices, Indices);
}

DrawUniformBuffer* engine::graphics::OpenGLRenderer::CreateUniformBuffer(size_t Size)
{
	return new OpenGLDrawUniformBuffer(Size, this);
}

ShaderProgramObject* engine::graphics::OpenGLRenderer::CreateShaderProgramObject(const string& Source, ShaderProgramType Type)
{
	try
	{
		return new OpenGLShaderProgramObject(Source, Type);
	}
	catch (ShaderCompilerError& e)
	{
		return nullptr;
	}
}

ShaderProgram* engine::graphics::OpenGLRenderer::LinkShaderProgram(std::vector<ShaderProgramObject*> Objects)
{
	try
	{
		return new OpenGLShaderProgram(Objects, this);
	}
	catch (ShaderCompilerError& e)
	{
		return nullptr;
	}
}

void engine::graphics::OpenGLRenderer::ActivateFramebuffer(uint32 BufferObject)
{
	if (this->ActiveFramebuffer != BufferObject)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, BufferObject);
		ActiveFramebuffer = BufferObject;
	}
}

void engine::graphics::OpenGLRenderer::ActivateTexture(uint32 TextureObject, uint8 Slot)
{
	ActivateTexture(TextureObject, Slot, GL_TEXTURE_2D);
}

void engine::graphics::OpenGLRenderer::ActivateTexture(uint32 TextureObject, uint8 Slot, uint32 Type)
{
	if (this->ActiveTextureSlot != Slot)
	{
		glActiveTexture(GL_TEXTURE0 + Slot);
		ActiveTextureSlot = Slot;
		ActiveTextureObject = 0;
	}

	if (this->ActiveTextureObject != TextureObject)
	{
		glBindTexture(Type, TextureObject);
		ActiveTextureObject = TextureObject;
	}
}

void engine::graphics::OpenGLRenderer::SetRenderResolution(uint32 NewWidth, uint32 NewHeight)
{
	if (NewWidth != ViewportWidth || NewHeight != ViewportHeight)
	{
		glViewport(0, 0, NewWidth, NewHeight);
		this->ViewportWidth = NewWidth;
		this->ViewportHeight = NewHeight;
	}
}

void engine::graphics::OpenGLRenderer::SetFaceCullEnabled(bool NewEnabled)
{
	if (NewEnabled != FaceCullEnabled)
	{
		if (NewEnabled)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		this->FaceCullEnabled = NewEnabled;
	}
}

void engine::graphics::OpenGLRenderer::SetDepthCheckEnabled(bool NewEnabled)
{
	if (NewEnabled != DepthTestEnabled)
	{
		if (NewEnabled)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		this->DepthTestEnabled = NewEnabled;
	}
}

void engine::graphics::OpenGLRenderer::SetBlendEnabled(bool NewEnabled)
{
	if (NewEnabled != BlendEnabled)
	{
		if (NewEnabled)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		this->BlendEnabled = NewEnabled;
	}
}

void engine::graphics::OpenGLRenderer::SetStencilEnabled(bool NewEnabled, uint8 Mask)
{
	if (NewEnabled)
	{
		SetStencilMask(Mask);
	}
	if (NewEnabled != this->StencilWriteEnabled)
	{
		if (NewEnabled)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		StencilWriteEnabled = NewEnabled;
	}
}

void engine::graphics::OpenGLRenderer::SetStencilValue(uint8 NewValue, uint8 Mask)
{
	SetStencilMask(Mask);
	glStencilFunc(GL_GEQUAL, GLint(NewValue), Mask);
}

void engine::graphics::OpenGLRenderer::SetStencilMask(uint8 Mask)
{
	if (this->StencilTestValue != Mask)
	{
		glStencilMask(Mask);
		this->StencilTestValue = Mask;
	}
}

void engine::graphics::OpenGLRenderer::UseProgram(uint32 NewProgram)
{
	if (CurrentShaderProgram != NewProgram)
	{
		glUseProgram(NewProgram);
		CurrentShaderProgram = NewProgram;
	}
}

static void GLAPIENTRY MessageCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR
		|| type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
		|| type == GL_DEBUG_TYPE_PORTABILITY)
	{
		((VideoSubsystem*)userParam)->Print(message, subsystem::Subsystem::LogType::Error);
	}
}

engine::graphics::OpenGLRenderer::OpenGLRenderer()
{
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, this);
}

void engine::graphics::OpenGLRenderer::RenderScreen(kui::Window* WithWindow, RendererTexture* Texture, bool VSync)
{
	SetRenderResolution(WithWindow->GetSize().X, WithWindow->GetSize().Y);
	SetFaceCullEnabled(false);
	SetDepthCheckEnabled(false);
	SetBlendEnabled(false);
	ActivateFramebuffer(0);
	SetStencilValue(0, 0);
	SetStencilEnabled(false, 0);

	auto GLTexture = static_cast<OpenGLRendererTexture*>(Texture);
	auto PostProcess = WithWindow->Shaders.LoadShader("shader/internal/postProcess.vert",
		"shader/internal/drawToWindow.frag", "engineToWindow");

	PostProcess->Bind();
	glActiveTexture(GL_TEXTURE0);

	if (GLTexture)
	{
		glBindTexture(GL_TEXTURE_2D, GLTexture->TextureObject);
	}
	auto GlRender = static_cast<render::OpenGLBackend*>(WithWindow->UI.Render);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GlRender->UITextures[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, GlRender->UITextures[1]);

	PostProcess->SetInt("u_texture", 0);
	PostProcess->SetInt("u_ui", 1);
	PostProcess->SetInt("u_alpha", 2);

#if EDITOR
	if (editor::EditorSubsystem::Active)
	{
		editor::Viewport* View = editor::Viewport::Current;

		PostProcess->SetVec2("u_pos", View->ViewportBackground->GetScreenPosition() / 2 + 0.5);
		PostProcess->SetVec2("u_size", View->ViewportBackground->GetUsedSize().GetScreen() / 2);
	}
	else
#endif
	{
		PostProcess->SetVec2("u_pos", 0);
		PostProcess->SetVec2("u_size", 1);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
	if (VSyncEnabled != VSync)
	{
		SDL_GL_SetSwapInterval(VSync ? 1 : 0);
		VSyncEnabled = VSync;
	}
	SDL_GL_SwapWindow(GetSysWindow(WithWindow)->SDLWindow);
	ActiveTextureSlot = 32;
	ActiveTextureObject = UINT32_MAX;
	ActiveFramebuffer = UINT32_MAX;
	CurrentShaderProgram = UINT32_MAX;
}

engine::graphics::OpenGLImageRendererTexture::OpenGLImageRendererTexture(const uByte* Pixels, uint32 Width,
	uint32 Height, const TextureOptions& Options, OpenGLRenderer* Render)
	: OpenGLRendererTexture(Render)
{
	this->Render = Render;
	this->TypeEnum = GL_TEXTURE_2D;
	glGenTextures(1, &TextureObject);
	Render->ActivateTexture(TextureObject, 0);

	GLint WrapMode = GL_CLAMP_TO_BORDER;

	switch (Options.TextureBorders)
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
	GLint Filter = Options.Filter == TextureOptions::Linear ? GL_LINEAR : GL_NEAREST;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Options.MipMaps ? GL_LINEAR_MIPMAP_LINEAR : Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);

	auto Alpha = Options.HasAlphaChannel;

	glTexImage2D(GL_TEXTURE_2D, 0, Alpha ? GL_RGBA8 : GL_RGB8, GLsizei(Width), GLsizei(Height), 0,
		Alpha ? GL_RGBA : GL_RGB, Options.IsFloatTexture ? GL_FLOAT : GL_UNSIGNED_BYTE, Pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
}

engine::graphics::OpenGLImageRendererTexture::~OpenGLImageRendererTexture()
{
	glDeleteTextures(1, &TextureObject);
}

uint32 engine::graphics::OpenGLRendererTexture::GetUITexture()
{
	return TextureObject;
}

engine::graphics::OpenGLRendererDrawTarget::OpenGLRendererDrawTarget(uint32 Width, uint32 Height,
	std::vector<DrawTargetBuffer> Buffers, OpenGLRenderer* Render)
{
	this->Width = Width;
	this->Height = Height;

	this->Render = Render;
	glGenFramebuffers(1, &Buffer);

	std::vector<uint32> NewTextures;
	NewTextures.resize(Buffers.size());

	glGenTextures(NewTextures.size(), NewTextures.data());
	Render->ActivateFramebuffer(Buffer);

	size_t AttachmentCounter = GL_COLOR_ATTACHMENT0;

	std::vector<uint32> DrawAttachments;

	for (size_t i = 0; i < Buffers.size(); i++)
	{
		auto t = TargetTexture(NewTextures[i], Render);

		Render->ActivateTexture(NewTextures[i], 0);

		switch (Buffers[i].Type)
		{
		case DrawTargetBufferType::Color:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, GLsizei(Width), GLsizei(Height), 0, GL_RGB, GL_FLOAT, 0);
			break;
		case DrawTargetBufferType::DepthStencil:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, GLsizei(Width), GLsizei(Height), 0,
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
			t.IsDepthStencil = true;
			StencilIndex = i;
			break;
		case DrawTargetBufferType::Depth:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, GLsizei(Width), GLsizei(Height), 0,
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
			break;
		}

		GLint Filtering = Buffers[i].FilterNearest ? GL_NEAREST : GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (Buffers[i].Type == DrawTargetBufferType::Color)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentCounter, GL_TEXTURE_2D, NewTextures[i], 0);
			DrawAttachments.push_back(AttachmentCounter);
			AttachmentCounter++;
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, NewTextures[i], 0);
		}
		this->Textures.push_back(t);
	}
	glDrawBuffers(DrawAttachments.size(), DrawAttachments.data());
}

engine::graphics::OpenGLRendererDrawTarget::~OpenGLRendererDrawTarget()
{
	std::vector<uint32> OldTextures;
	OldTextures.reserve(this->Textures.size());

	for (auto& i : this->Textures)
	{
		OldTextures.push_back(i.TextureObject);
	}

	glDeleteFramebuffers(1, &Buffer);
	glDeleteTextures(OldTextures.size(), OldTextures.data());
}

RendererTexture* engine::graphics::OpenGLRendererDrawTarget::GetTexture(size_t Index)
{
	if (this->Textures[Index].IsDepthStencil)
	{
		this->Textures[Index].SetStencilMode(false);
	}
	return &this->Textures[Index];
}

engine::graphics::OpenGLRendererDrawTarget::TargetTexture::TargetTexture(uint32 t, OpenGLRenderer* Render)
	: OpenGLRendererTexture(Render)
{
	this->TextureObject = t;
	this->TypeEnum = GL_TEXTURE_2D;
}

void engine::graphics::OpenGLRendererDrawTarget::TargetTexture::SetStencilMode(bool NewIsStencilMode)
{
	if (this->IsStencilMode != NewIsStencilMode)
	{
		Render->ActivateTexture(this->TextureObject, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE,
			NewIsStencilMode ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
		IsStencilMode = NewIsStencilMode;
	}
}

RendererTexture* engine::graphics::OpenGLRendererDrawTarget::GetStencilTexture()
{
	if (StencilIndex != SIZE_MAX)
	{
		this->Textures[StencilIndex].SetStencilMode(true);
		return &this->Textures[StencilIndex];
	}
	return nullptr;
}

void engine::graphics::OpenGLRendererDrawTarget::TargetTexture::SetFilterMode(TextureOptions::Filtering NewFilter,
	size_t TextureIndex)
{
	auto Filter = NewFilter == TextureOptions::Nearest ? GL_NEAREST : GL_LINEAR;
	Render->ActivateTexture(TextureObject, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
}

void engine::graphics::OpenGLRendererTexture::SetFilterMode(TextureOptions::Filtering NewFilter, size_t TextureIndex)
{
	auto Filter = NewFilter == TextureOptions::Nearest ? GL_NEAREST : GL_LINEAR;
	Render->ActivateTexture(TextureObject, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
}

void engine::graphics::OpenGLRendererDrawTarget::Activate()
{
	Render->FramebufferWidth = this->Width;
	Render->FramebufferHeight = this->Height;
	Render->ActivateFramebuffer(Buffer);
}

void engine::graphics::OpenGLRendererDrawTarget::Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil)
{
	if (!ClearColor && !ClearDepth && !ClearStencil)
	{
		return;
	}

	Render->SetRenderResolution(Width, Height);
	uint32 Flags = 0;

	if (ClearColor)
	{
		Flags |= GL_COLOR_BUFFER_BIT;
	}
	if (ClearDepth)
	{
		Flags |= GL_DEPTH_BUFFER_BIT;
	}
	if (ClearStencil)
	{
		Render->SetStencilMask(ClearStencil);
		Flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(Flags);
}

engine::graphics::OpenGLVertexBuffer::OpenGLVertexBuffer(const std::vector<Vertex>& Vertices,
	const std::vector<uint32>& Indices)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(uint32), Indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));

	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glBindVertexArray(0);

	IndicesSize = uint32(Indices.size());
}

engine::graphics::OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void engine::graphics::OpenGLVertexBuffer::Draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, IndicesSize, GL_UNSIGNED_INT, 0);
}

engine::graphics::OpenGLDrawCommand::OpenGLDrawCommand(OpenGLRenderer* Renderer)
{
	this->Render = Renderer;

	this->ViewportWidth = Renderer->FramebufferWidth;
	this->ViewportHeight = Renderer->FramebufferHeight;
}

void engine::graphics::OpenGLDrawCommand::UseShader(ShaderObject* TargetShader)
{
	this->CurrentShader = TargetShader;
	TargetShader->Bind();
}

void engine::graphics::OpenGLDrawCommand::BindTexture(const string& UniformName, RendererTexture* Target)
{
	BindTexture(CurrentShader->GetUniformLocation(UniformName), Target);
}

void engine::graphics::OpenGLDrawCommand::BindTexture(uint32 UniformLocation, RendererTexture* Target)
{
	auto GLTexture = static_cast<OpenGLRendererTexture*>(Target);

	if (GLTexture)
	{
		Render->ActivateTexture(GLTexture->TextureObject, TextureSlotCounter, GLTexture->TypeEnum);
	}
	else
	{
		Render->ActivateTexture(0, TextureSlotCounter);
	}
	CurrentShader->SetInt(UniformLocation, TextureSlotCounter);
	TextureSlotCounter++;
}

void engine::graphics::OpenGLDrawCommand::BindUniformBlock(const string& UniformBlockName, DrawUniformBuffer* Target)
{
	auto GLUniformBlock = static_cast<OpenGLDrawUniformBuffer*>(Target);
	auto GLShader = static_cast<OpenGLShaderProgram*>(CurrentShader->Program);

	glUniformBlockBinding(GLShader->ProgramObject,
		CurrentShader->GetUniformBlockLocation(UniformBlockName), GLUniformBlock->BufferId);
}

void engine::graphics::OpenGLDrawCommand::DrawVertices(size_t Count)
{
	Apply();
	glDrawArrays(GL_TRIANGLES, 0, Count);
}

void engine::graphics::OpenGLDrawCommand::DrawVertexBuffer(VertexBuffer* Buffer)
{
	Apply();
	Buffer->Draw();
}

void engine::graphics::OpenGLDrawCommand::ResetTextures()
{
	this->TextureSlotCounter = 1;
}

void engine::graphics::OpenGLDrawCommand::SetResolution(uint32 Width, uint32 Height)
{
	this->ViewportWidth = Width;
	this->ViewportHeight = Height;
}

void engine::graphics::OpenGLDrawCommand::SetFaceCullEnabled(bool NewEnabled)
{
	FaceCullEnabled = NewEnabled;
}

void engine::graphics::OpenGLDrawCommand::SetDepthCheckEnabled(bool NewEnabled)
{
	DepthCheckEnabled = NewEnabled;
}

void engine::graphics::OpenGLDrawCommand::SetBlendEnabled(bool NewEnabled)
{
	BlendEnabled = NewEnabled;
}

void engine::graphics::OpenGLDrawCommand::SetStencilValue(bool Enabled, uint8 Value)
{
	StencilValue = Value;
	StencilEnabled = Enabled;
}

void engine::graphics::OpenGLDrawCommand::Reset()
{
	this->CurrentShader = nullptr;
	this->TextureSlotCounter = 1;
}

void engine::graphics::OpenGLDrawCommand::Apply()
{
	Render->SetFaceCullEnabled(FaceCullEnabled);
	Render->SetDepthCheckEnabled(DepthCheckEnabled);
	Render->SetRenderResolution(ViewportWidth, ViewportHeight);
	Render->SetBlendEnabled(BlendEnabled);
	Render->SetStencilEnabled(StencilEnabled, StencilEnabled ? 0xff : 0);
	Render->SetStencilValue(StencilValue, StencilEnabled ? 0xff : 0);
}

uint32 engine::graphics::OpenGLRendererDrawTarget::TargetTexture::GetUITexture()
{
	return TextureObject;
}

engine::graphics::RendererShadowDrawTarget::RendererShadowDrawTarget(uint32 Width, uint32 Height, uint32 Count, OpenGLRenderer* Render)
	: OpenGLRendererTexture(Render)
{
	this->Width = Width;
	this->Height = Height;
	this->Render = Render;
	TypeEnum = GL_TEXTURE_2D_ARRAY;

	glGenFramebuffers(1, &LightFBO);

	glGenTextures(1, &TextureObject);
	Render->ActivateTexture(TextureObject, 0, TypeEnum);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_DEPTH_COMPONENT32F,
		Width,
		Height,
		GLsizei(Count),
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	Render->ActivateFramebuffer(LightFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TextureObject, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::Error("Failed to create the shadow framebuffer!");
		return;
	}
}

RendererTexture* engine::graphics::RendererShadowDrawTarget::GetTexture(size_t Index)
{
	return this;
}

RendererTexture* engine::graphics::RendererShadowDrawTarget::GetStencilTexture()
{
	return nullptr;
}

void engine::graphics::RendererShadowDrawTarget::Activate()
{
	Render->FramebufferWidth = this->Width;
	Render->FramebufferHeight = this->Height;
	Render->ActivateFramebuffer(LightFBO);
}

void engine::graphics::RendererShadowDrawTarget::Clear(bool ClearColor, bool ClearDepth, uint8 ClearStencil)
{
	if (!ClearColor && !ClearDepth && !ClearStencil)
	{
		return;
	}

	Render->SetRenderResolution(Width, Height);
	uint32 Flags = 0;

	if (ClearColor)
	{
		Flags |= GL_COLOR_BUFFER_BIT;
	}
	if (ClearDepth)
	{
		Flags |= GL_DEPTH_BUFFER_BIT;
	}
	if (ClearStencil)
	{
		Flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(Flags);
}

engine::graphics::OpenGLDrawUniformBuffer::OpenGLDrawUniformBuffer(size_t Size, OpenGLRenderer* Render)
{
	this->Render = Render;
	BufferId = Render->UniformBufferIndex++;
	glGenBuffers(1, &BufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, BufferObject);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, BufferId, BufferObject);
}

void engine::graphics::OpenGLDrawUniformBuffer::Write(size_t Offset, void* Data, size_t DataSize)
{
	glBufferSubData(GL_UNIFORM_BUFFER, Offset, DataSize, Data);
}

bool engine::graphics::OpenGLShaderProgram::CheckCompileErrors(uint32 ShaderID, string Type)
{
	GLint success;
	GLchar infoLog[1024];
	if (Type != "Link")
	{
		glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ShaderID, 1024, NULL, infoLog);
			Log::Error("Shader compilation error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog));
			return true;
		}
	}
	else
	{
		glGetProgramiv(ShaderID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ShaderID, 1024, NULL, infoLog);
			Log::Error("Shader linking error of type: "
				+ Type
				+ "\n"
				+ std::string(infoLog));
			return true;
		}
	}
	return false;
}

engine::graphics::OpenGLShaderProgram::OpenGLShaderProgram(std::vector<ShaderProgramObject*> Objects, OpenGLRenderer* Render)
{
	this->Render = Render;
	ProgramObject = glCreateProgram();
	for (ShaderProgramObject* Object : Objects)
	{
		glAttachShader(ProgramObject, static_cast<OpenGLShaderProgramObject*>(Object)->CompiledObject);
	}

	glLinkProgram(ProgramObject);

	if (CheckCompileErrors(ProgramObject, "Link"))
	{
		throw ShaderCompilerError();
	}
}

engine::graphics::OpenGLShaderProgram::~OpenGLShaderProgram()
{
	glDeleteProgram(ProgramObject);
}

uint32 engine::graphics::OpenGLShaderProgram::GetUniformLocation(const char* Name)
{
	return glGetUniformLocation(this->ProgramObject, Name);
}

uint32 engine::graphics::OpenGLShaderProgram::GetUniformBlockLocation(const char* Name)
{
	return glGetUniformBlockIndex(ProgramObject, Name);
}

void engine::graphics::OpenGLShaderProgram::SetInt(uint32 UniformLocation, int32 Value)
{
	Activate();
	glUniform1i(UniformLocation, Value);
}

void engine::graphics::OpenGLShaderProgram::SetFloat(uint32 UniformLocation, float Value)
{
	Activate();
	glUniform1f(UniformLocation, Value);
}

void engine::graphics::OpenGLShaderProgram::SetVec3(uint32 UniformLocation, Vector3 Value)
{
	Activate();
	glUniform3f(UniformLocation, Value.X, Value.Y, Value.Z);
}

void engine::graphics::OpenGLShaderProgram::SetVec2(uint32 UniformLocation, Vector2 Value)
{
	Activate();
	glUniform2f(UniformLocation, Value.X, Value.Y);
}

void engine::graphics::OpenGLShaderProgram::SetMatrix(uint32 UniformLocation, const glm::mat4& Value)
{
	Activate();
	glUniformMatrix4fv(UniformLocation, 1, false, &Value[0][0]);
}

void engine::graphics::OpenGLShaderProgram::Activate()
{
	if (Render->CurrentShaderProgram != ProgramObject)
	{
		glUseProgram(ProgramObject);
		Render->CurrentShaderProgram = ProgramObject;
	}
}

engine::graphics::OpenGLShaderProgramObject::OpenGLShaderProgramObject(const string& Source, ShaderProgramType Type)
{
	uint32 GLType = 0;
	string TypeName = "";

	switch (Type)
	{
	case ShaderProgramType::Vertex:
		GLType = GL_VERTEX_SHADER;
		TypeName = "Vertex";
		break;
	case ShaderProgramType::Fragment:
		GLType = GL_FRAGMENT_SHADER;
		TypeName = "Fragment";
		break;
	case ShaderProgramType::Geometry:
		GLType = GL_GEOMETRY_SHADER;
		TypeName = "Geometry";
		break;
	}

	CompiledObject = glCreateShader(GLType);
	const char* SourcePointer = Source.c_str();
	glShaderSource(CompiledObject, 1, &SourcePointer, nullptr);
	glCompileShader(CompiledObject);
	if (OpenGLShaderProgram::CheckCompileErrors(CompiledObject, "SHADER"))
	{
		throw ShaderCompilerError();
	}
}

engine::graphics::OpenGLShaderProgramObject::~OpenGLShaderProgramObject()
{
	glDeleteShader(CompiledObject);
}

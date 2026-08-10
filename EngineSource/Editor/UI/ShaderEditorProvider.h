#pragma once
#include <Editor/UI/EngineTextEditorProvider.h>
#include <Engine/Graphics/ShaderModules.h>

namespace engine::editor
{
	class ShaderEditorProvider : public EngineTextEditorProvider
	{
	public:

		ShaderEditorProvider(string FilePath, kui::Vec3f VariableColor, kui::Vec3f FunctionColor);

		void Reload();

		kui::Vec3f VariableColor = 0.5f;
		kui::Vec3f FunctionColor = 0.5f;

	private:
		bool IsFragment = false;
		graphics::ShaderModuleLoader::Result LoadedShader;
	};
}
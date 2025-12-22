#include "ScriptEditorContext.h"
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>

using namespace ds;
using namespace kui;

engine::editor::ScriptEditorContext::ScriptEditorContext()
{
	this->ScriptService = Engine::GetSubsystem<script::ScriptSubsystem>()
		->ScriptLanguage->startService();

	this->ScriptService->parser->errors.errorCallback = [this]
	(ErrorCode code, std::string File, const Token& At, std::string Description)
	{
		this->Errors[File].push_back(ScriptError{
			.At = EditorPosition(At.position.startPos, At.position.line),
			.Length = At.position.endPos - At.position.startPos,
			.Description = Description,
			});
	};

}

void engine::editor::ScriptEditorContext::AddFile(const std::string& Content, const std::string& Name)
{
	this->ScriptService->addString(Content, Name);
}

void engine::editor::ScriptEditorContext::UpdateFile(const std::string& Content, const std::string& Name)
{
	Errors[Name].clear();
	ScriptService->updateFile(Content, Name);
}

void engine::editor::ScriptEditorContext::Commit()
{
	ScriptService->commitChanges();
}

void engine::editor::ScriptEditorContext::Initialize()
{
	Commit();
	OnReady.Invoke();
	Loaded = true;
}
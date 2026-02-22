#include "ScriptUICanvas.h"
#include <ds/interpreter.hpp>
#include <Engine/Script/ScriptSubsystem.h>

engine::script::ui::ScriptUICanvas::ScriptUICanvas(ds::RuntimeClass* ScriptObject)
{
	this->ScriptObject = ScriptObject;
	this->ScriptObject->addRef();
	CanvasBox->DeleteChildren();

	auto Scripts = ScriptSubsystem::Instance;

	MarkupBox = new kui::markup::UIDynMarkupBox(&Scripts->UIContext,
		Scripts->UIData.ClassIdMappings.at(ScriptObject->type));
	CanvasBox->AddChild(MarkupBox);
	CanvasBox->UpdateElement();
	CanvasBox->RedrawElement();
}

engine::script::ui::ScriptUICanvas::~ScriptUICanvas()
{
	ScriptSubsystem::Instance->Runtime->baseContext.destruct(ScriptObject);
}

void engine::script::ui::ScriptUICanvas::Update()
{
	if (Engine::IsPlaying && ScriptObject && ScriptObject->vtable && ScriptObject->vtable[1])
	{
		ScriptSubsystem::Instance->Runtime->baseContext.pushValue(this->ScriptObject);
		ScriptSubsystem::Instance->Runtime->baseContext.virtualCall(ScriptObject->vtable[1]);
	}
}

#include "ScriptUICanvas.h"
#include <ds/interpreter.hpp>
#include <Engine/Script/ScriptSubsystem.h>

engine::script::ui::ScriptUICanvas::ScriptUICanvas(ds::RuntimeClass* ScriptObject)
{
	this->ScriptObject = ScriptObject;
	this->ScriptObject->addRef();
	CanvasBox->DeleteChildren();

	MarkupBox = new kui::markup::UIDynMarkupBox(&ScriptSubsystem::Instance->UIContext, "PlayerUI");
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
	if (Engine::IsPlaying && ScriptObject && ScriptObject->vtable[1])
	{
		ScriptSubsystem::Instance->Runtime->baseContext.pushValue(this->ScriptObject);
		ScriptSubsystem::Instance->Runtime->baseContext.virtualCall(ScriptObject->vtable[1]);
	}
}

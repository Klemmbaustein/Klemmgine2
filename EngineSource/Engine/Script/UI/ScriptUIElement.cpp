#include "ScriptUIElement.h"
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/interpreter.hpp>

engine::script::ui::ScriptUIElement::ScriptUIElement(ds::RuntimeClass* ScriptObject)
{
	this->ScriptObject = ScriptObject;
	this->ScriptObject->addRef();

	auto Scripts = ScriptSubsystem::Instance;

	Load(&Scripts->UIContext, Scripts->UIData.ClassIdMappings.at(ScriptObject->type));
	Scripts->UIObjectMappings[this] = ScriptObject;
}

engine::script::ui::ScriptUIElement::~ScriptUIElement()
{
	auto Scripts = ScriptSubsystem::Instance;
	Scripts->UIObjectMappings.erase(this);
	Scripts->Runtime->baseContext.destruct(ScriptObject);
}

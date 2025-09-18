#include "ScriptObject.h"
#include <interpreter.hpp>

using namespace lang;

engine::script::ScriptObject::ScriptObject(const lang::TypeInfo& Class,
	lang::InterpretContext* Interpreter)
	: Class(Class), Interpreter(Interpreter)
{
	LoadScriptData();
}

void engine::script::ScriptObject::Begin()
{
	if (!this->ScriptData)
	{
		this->ScriptData = Class.create(Interpreter);
		ClassRef<ScriptObjectData*> Data = this->ScriptData;

		Data.getValue() = new ScriptObjectData{
			.Parent = this,
			.Position = this->Position,
		};
	}

	for (auto& i : this->Properties)
	{
		i->OnChanged();
	}

	if (ScriptData->vtable[1])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[1]);
	}
}

void engine::script::ScriptObject::Update()
{
	if (ScriptData && ScriptData->vtable[3])
	{
		auto Data = reinterpret_cast<ScriptObjectData*>(this->ScriptData->getBody());
		Data->Position = this->Position;
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[3]);
		this->Position = Data->Position;
	}
}

void engine::script::ScriptObject::LoadScriptData()
{
	if (this->ScriptData)
	{
		delete this->ScriptData;
	}

	this->ScriptData = Class.create(Interpreter);
	ClassRef<ScriptObjectData*> Data = this->ScriptData;

	Data.getValue() = new ScriptObjectData{
		.Parent = this
	};

	for (auto& i : this->Properties)
	{
		delete i;
	}
	Properties.clear();

	for (auto& i : this->Class.members)
	{
		RuntimeStrRef MemberValue = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

		auto p = new ObjProperty<string>(i.name, MemberValue.ptr(), this);

		p->OnChanged = [this, i, p]
		{
			RuntimeClass*& MemberValue = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

			RuntimeStrRef NewString = RuntimeStrRef(p->Value.data(), p->Value.size());
			MemberValue = NewString.classPtr;
		};
	}
}

void engine::script::ScriptObject::UnloadScriptData()
{
	Interpreter->destruct(ScriptData);
	this->ScriptData = nullptr;
}

void engine::script::ScriptObject::OnDestroyed()
{
	if (ScriptData->vtable[2])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[2]);
	}
	UnloadScriptData();
}

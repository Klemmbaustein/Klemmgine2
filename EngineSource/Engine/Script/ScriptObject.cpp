#include "ScriptObject.h"
#include <interpreter.hpp>

using namespace lang;

engine::script::ScriptObject::ScriptObject(const lang::TypeInfo& Class,
	lang::InterpretContext* Interpreter)
	: Class(Class), Interpreter(Interpreter)
{
	this->ScriptData = Class.create(Interpreter);
	ClassRef<ScriptObjectData*> Data = this->ScriptData;

	Data.getValue() = new ScriptObjectData{
		.Parent = this
	};

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

void engine::script::ScriptObject::Begin()
{
	if (!this->ScriptData)
	{
		this->ScriptData = Class.create(Interpreter);
		ClassRef<ScriptObjectData*> Data = this->ScriptData;

		Data.getValue() = new ScriptObjectData{
			.Parent = this
		};
	}

	for (auto& i : this->Properties)
	{
		i->OnChanged();
	}

	Interpreter->pushValue(this->ScriptData);
	Interpreter->virtualCall(ScriptData->vtable[1]);
}

void engine::script::ScriptObject::Update()
{
	if (ScriptData->vtable[3])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[3]);
	}
}

void engine::script::ScriptObject::OnDestroyed()
{
	Interpreter->pushValue(this->ScriptData);
	Interpreter->virtualCall(ScriptData->vtable[2]);
	Interpreter->destruct(ScriptData);
	this->ScriptData = nullptr;
}

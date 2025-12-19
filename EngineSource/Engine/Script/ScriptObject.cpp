#include "ScriptObject.h"
#include "EngineModules.h"
#include "Engine/Engine.h"
#include <ds/modules/system.async.hpp>
#include <ds/interpreter.hpp>

using namespace ds;
using ds::modules::system::async::Task;

engine::script::ScriptObject::ScriptObject(const ds::TypeInfo& Class,
	ds::InterpretContext* Interpreter)
	: Class(Class), Interpreter(Interpreter)
{
	LoadScriptData();
}

void engine::script::ScriptObject::Begin()
{
	if (!this->ScriptData)
	{
		InitializeScriptPointer();
	}

	for (auto& i : this->Properties)
	{
		i->OnChanged();
	}

	Interpreter->callVirtualMethodVoid(ScriptData, 1);
}

void engine::script::ScriptObject::Update()
{
	if (Engine::IsPlaying && ScriptData && ScriptData->vtable[3])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[3]);
	}
}

void engine::script::ScriptObject::LoadScriptData()
{
	if (this->ScriptData)
	{
		delete this->ScriptData;
	}

	InitializeScriptPointer();

	for (auto& i : this->Properties)
	{
		delete i;
	}
	Properties.clear();

	for (auto& i : this->Class.members)
	{
		auto& member = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

		if (!member)
		{
			member = engine::script::CreateAssetRef();
		}

		ClassRef<AssetRef*> MemberValue = member;

		auto p = new ObjProperty<AssetRef>(i.name, *MemberValue.getValue(), this);

		p->OnChanged = [this, i, p]
		{
			auto& member = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);
			if (!member)
			{
				member = engine::script::CreateAssetRef();
			}

			ClassRef<AssetRef*> MemberValue = member;
			*MemberValue.getValue() = p->Value;
		};
	}
}

void engine::script::ScriptObject::UnloadScriptData()
{
	Interpreter->destruct(ScriptData);
	this->ScriptData = nullptr;
}

void engine::script::ScriptObject::InitializeScriptPointer()
{
	this->ScriptData = Class.create(Interpreter);
	ClassRef<SceneObject*> ScriptDataRef = this->ScriptData;
	ScriptDataRef.getValue() = this;
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

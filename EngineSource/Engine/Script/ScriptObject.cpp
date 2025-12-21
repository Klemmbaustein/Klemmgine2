#include "ScriptObject.h"
#include "EngineModules.h"
#include "Engine/Engine.h"
#include "ScriptSubsystem.h"
#include <Engine/Engine.h>
#include <ds/modules/system.async.hpp>
#include <ds/interpreter.hpp>
#include <ds/parser/types/stringType.hpp>

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
	auto Script = Engine::GetSubsystem<ScriptSubsystem>();

	if (this->ScriptData)
	{
		delete this->ScriptData;
	}

	InitializeScriptPointer();

	std::map<string, ObjPropertyBase*> OldProperties;

	for (auto& i : this->Properties)
	{
		OldProperties.insert({i->Name, i});
	}
	Properties.clear();

	for (auto& i : this->Class.members)
	{
		if (i.type == Script->ScriptEngine.AssetRefType)
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
		else if (i.type == Script->ScriptEngine.Vector3Type)
		{
			auto& member = *reinterpret_cast<Vector3*>(this->ScriptData->getBody() + i.offset);

			auto p = new ObjProperty<Vector3>(i.name, member, this);

			p->OnChanged = [this, i, p]
			{
				auto& member = *reinterpret_cast<Vector3*>(this->ScriptData->getBody() + i.offset);
				member = p->Value;
			};
		}
		else if (i.type == StringType::getInstance()->id)
		{
			RuntimeStrRef str = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

			if (!str.classPtr)
			{
				str = RuntimeStrRef("", 0);
			}

			auto p = new ObjProperty<string>(i.name, string(str.ptr(), str.length()), this);

			p->OnChanged = [this, i, p]
			{
				auto& memberPtr = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);
				RuntimeStrRef str = RuntimeStrRef(p->Value.c_str(), p->Value.size());

				memberPtr = str.classPtr;
			};
		}
		else
		{
			Log::Warn(str::Format("The property %s has an unsupported type.", i.name.c_str()));
		}
	}

	for (auto& i : this->Properties)
	{
		auto Found = OldProperties.find(i->Name);

		if (Found != OldProperties.end())
		{
			auto s = Found->second->Serialize();
			i->DeSerialize(&s);
			i->OnChanged();
		}
	}

	for (auto& i : OldProperties)
	{
		delete i.second;
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

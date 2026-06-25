#include <Engine/Script/ScriptSceneObject.h>
#include "Engine/Engine.h"
#include <Engine/Script/Bindings/AssetBindings.h>
#include <ds/parser/types/stringType.hpp>
#include "ScriptSubsystem.h"

using namespace ds;

void engine::script::ScriptSceneObject::OnDestroyed()
{
	if (ScriptData  && ScriptData->vtable[2])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[3]);
	}
	UnloadScriptData();
}

engine::script::ScriptSceneObject::ScriptSceneObject(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter)
	: ScriptObject(Class, Interpreter)
{
	ScriptObject::LoadScriptData();

	LoadProperties();
}

void engine::script::ScriptSceneObject::Begin()
{
	if (!ScriptData)
	{
		InitializeScriptPointer();
	}

	// If it failed to load the script data, give up
	if (!ScriptData)
	{
		return;
	}

	for (auto& i : this->Properties)
	{
		i->OnChanged();
	}

	ScriptSubsystem::Instance->RegisterClassForObject(this, ScriptData);

	Interpreter->callVirtualMethodVoid(ScriptData, 1);
	if (Engine::IsPlaying)
	{
		Interpreter->callVirtualMethodVoid(ScriptData, 2);
	}
}

void engine::script::ScriptSceneObject::Update()
{
	if (Engine::IsPlaying && ScriptData && ScriptData->vtable[4])
	{
		Interpreter->pushValue(this->ScriptData);
		Interpreter->virtualCall(ScriptData->vtable[4]);
	}
}

void engine::script::ScriptSceneObject::BeginHotReload()
{
	OnDestroyed();
	OnDestroyedEvent.Invoke();
	ClearComponents();
}

void engine::script::ScriptSceneObject::EndHotReload(ds::ReflectInfo* ClassData)
{
	Class = ClassData->types[Class.hash];
	LoadScriptData();
	Begin();
}

void engine::script::ScriptSceneObject::InitializePropertyFlags(ObjPropertyBase* p, const string& FlagsString)
{
	std::vector<string> SplitString = str::Split(FlagsString, ",");

	for (const string& SubString : SplitString)
	{
		if (SubString == "color")
		{
			p->AddHint(PropertyHint::Vec3Color);
		}
		if (SubString == "rotation")
		{
			p->AddHint(PropertyHint::Vec3Rotation);
		}
	}
}

void engine::script::ScriptSceneObject::LoadProperties()
{
	if (!this->ScriptData)
	{
		return;
	}

	std::map<string, ObjPropertyBase*> OldProperties;

	for (auto& i : this->Properties)
	{
		OldProperties.insert({ i->Name, i });
	}

	auto Script = ScriptSubsystem::Instance;
	Properties.clear();

	for (auto& i : this->Class.members)
	{
		if (i.attributeType != Script->ScriptEngine.ExportAttributeType)
		{
			continue;
		}

		string Name = i.getParameterValue("name").value_or(i.name);
		string Hints = i.getParameterValue("hint").value_or("");
		bool Visible = i.getParameterValue("visible").value_or("true") == "true";

		if (i.type == Script->ScriptEngine.AssetRefType)
		{
			auto& member = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

			if (!member)
			{
				member = script::CreateAssetRef();
			}

			ClassRef<AssetRef*> MemberValue = member;

			auto p = new ObjProperty<AssetRef>(Name, *MemberValue.getValue(), this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				auto& member = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);
				if (!member)
				{
					member = script::CreateAssetRef();
				}

				ClassRef<AssetRef*> MemberValue = member;
				*MemberValue.getValue() = p->Value;
			};
		}
		else if (i.type == Script->ScriptEngine.Vector3Type)
		{
			auto& member = *reinterpret_cast<Vector3*>(this->ScriptData->getBody() + i.offset);

			auto p = new ObjProperty<Vector3>(Name, member, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				auto& member = *reinterpret_cast<Vector3*>(this->ScriptData->getBody() + i.offset);
				member = p->Value;
			};
		}
		else if (i.type == StringType::STRING_ID)
		{
			RuntimeStrRef str = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

			if (!str.classPtr)
			{
				str = RuntimeStrRef("", 0);
			}

			auto p = new ObjProperty<string>(Name, string(str.ptr(), str.length()), this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				auto& memberPtr = *reinterpret_cast<RuntimeClass**>(this->ScriptData->getBody() + i.offset);

				this->Interpreter->destruct(memberPtr);
				RuntimeStrRef str = RuntimeStrRef(p->Value.c_str(), p->Value.size());

				memberPtr = str.classPtr;
			};
		}
		else if (i.type == IntType::INT_ID)
		{
			ds::Int Value = *reinterpret_cast<ds::Int*>(this->ScriptData->getBody() + i.offset);

			auto p = new ObjProperty<int32>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				ds::Int& Value = *reinterpret_cast<ds::Int*>(this->ScriptData->getBody() + i.offset);

				Value = p->Value;
			};
		}
		else if (i.type == FloatType::FLOAT_ID)
		{
			ds::Float Value = *reinterpret_cast<ds::Float*>(this->ScriptData->getBody() + i.offset);

			auto p = new ObjProperty<float>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				ds::Float& Value = *reinterpret_cast<ds::Float*>(this->ScriptData->getBody() + i.offset);

				Value = p->Value;
			};
		}
		else
		{
			Log::Warn(str::Format("%s: The property '%s' has an unsupported type.",
				Class.name.c_str(), i.name.c_str()));
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

void engine::script::ScriptSceneObject::LoadScriptData()
{
	ScriptObject::LoadScriptData();

	LoadProperties();
}

void engine::script::ScriptSceneObject::InitializeScriptPointer()
{
	if (Class.hash)
	{
		InitializePointerWithValue<SceneObject*>(this);
	}
	else
	{
		ScriptData = nullptr;
	}
}

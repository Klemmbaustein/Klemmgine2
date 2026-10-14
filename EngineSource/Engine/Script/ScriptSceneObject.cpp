#include <Engine/Script/ScriptSceneObject.h>
#include "Engine/Engine.h"
#include <Engine/Script/Bindings/AssetBindings.h>
#include <ds/parser/types/stringType.hpp>
#include "ScriptSubsystem.h"

using namespace ds;

void engine::script::ScriptSceneObject::OnDestroyed()
{
	if (ScriptData  && ScriptData->vtable[3])
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

	(void)Interpreter->callVirtualMethodVoid(ScriptData, 1);
	if (Engine::IsPlaying)
	{
		(void)Interpreter->callVirtualMethodVoid(ScriptData, 2);
	}
}

void engine::script::ScriptSceneObject::Update()
{
	if (Engine::IsPlaying && ScriptData)
	{
		(void)Interpreter->callVirtualMethodVoid(ScriptData, 4);
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
}

void engine::script::ScriptSceneObject::ReInitializeAfterHotReload()
{
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
		if (i.attribute.type != Script->ScriptEngine.ExportAttributeType)
		{
			continue;
		}

		string Name = i.attribute.getParameterValue("name").value_or(i.name);
		string Hints = i.attribute.getParameterValue("hint").value_or("");
		bool Visible = i.attribute.getParameterValue("visible").value_or("true") == "true";

		if (i.type == Script->ScriptEngine.AssetRefType)
		{
			auto member = i.getFromInstance<RuntimeClass*>(ScriptData);

			if (!member)
			{
				member = script::CreateAssetRef();
				i.setClassToInstance(ScriptData, member, nullptr);
			}

			ClassRef<AssetRef*> MemberValue = member;

			auto p = new ObjProperty<AssetRef>(Name, *MemberValue.getValue(), this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				ClassRef<AssetRef*> newRef = script::CreateAssetRef();
				*newRef.getValue() = p->Value;

				i.setClassToInstance(ScriptData, newRef.classPtr, Interpreter);
			};
		}
		else if (i.type == Script->ScriptEngine.Vector3Type)
		{
			Vector3 Value = i.getFromInstance<Vector3>(ScriptData);

			auto p = new ObjProperty<Vector3>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				i.setToInstance(ScriptData, p->Value);
			};
		}
		else if (i.type == StringType::STRING_ID)
		{
			auto str = i.getFromInstance<RuntimeStrRef>(ScriptData);

			if (!str.classPtr)
			{
				str = RuntimeStrRef("", 0);
				i.setClassToInstance(ScriptData, str.classPtr, nullptr);
			}

			auto p = new ObjProperty<string>(Name, string(str.ptr(), str.length()), this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				RuntimeStrRef str = RuntimeStrRef(p->Value.c_str(), p->Value.size());

				i.setClassToInstance(ScriptData, str.classPtr, Interpreter);
			};
		}
		else if (i.type == IntType::INT_ID)
		{
			ds::Int Value = i.getFromInstance<Int>(ScriptData);

			auto p = new ObjProperty<int32>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				i.setToInstance(ScriptData, p->Value);
			};
		}
		else if (i.type == FloatType::FLOAT_ID)
		{
			ds::Float Value = i.getFromInstance<Int>(ScriptData);

			auto p = new ObjProperty<float>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				i.setToInstance(ScriptData, p->Value);
			};
		}
		else if (i.type == BoolType::BOOL_ID)
		{
			ds::Bool Value = i.getFromInstance<Bool>(ScriptData);

			auto p = new ObjProperty<bool>(Name, Value, this);
			p->IsHidden = !Visible;
			InitializePropertyFlags(p, Hints);

			p->OnChanged = [this, i, p] {
				i.setToInstance<Bool>(ScriptData, p->Value);
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

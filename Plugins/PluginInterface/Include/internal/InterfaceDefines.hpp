// Core
STRUCT_MEMBER(Log, void, (const char* Message), Log::Info(Message))

// Console

STRUCT_MEMBER(ConsoleExecuteCommand, void, (const char* cmd), console::ExecuteCommand(cmd); )

// Objects
STRUCT_MEMBER(RegisterObj, int32, (std::string Name, SceneObject* (*Func)(void* UserData), void* UserData), \
{ return Reflection::RegisterObject(Name, [Func, UserData]() -> SceneObject* { return Func(UserData); }); })

STRUCT_MEMBER(CreateObj, engine::SceneObject*, (engine::Scene* Scn, int32 TypeID, Vector3 pos, Rotation3 rot, Vector3 scl), \
{ \
if (!Scn) Scn = Scene::GetMain(); \
return Scn->CreateObjectFromID(TypeID, pos, rot, scl); \
})

STRUCT_MEMBER(GetObjName, const char*, (engine::SceneObject* Target), return Target->Name.c_str())

// Components
STRUCT_MEMBER(NewMeshComponent, void*, (), return new MeshComponent())
STRUCT_MEMBER(ObjectAttachComponent, void, (void* Obj, void* Comp), ((engine::SceneObject*)Obj)->Attach((ObjectComponent*)Comp))
STRUCT_MEMBER(ComponentAttach, void, (void* Parent, void* Comp), ((ObjectComponent*)Parent)->Attach((ObjectComponent*)Comp))
STRUCT_MEMBER(MeshComponentLoad, void, (void* Comp, const char* Name), ((MeshComponent*)Comp)->Load(AssetRef::Convert(Name)))

// Input
STRUCT_MEMBER(InputIsKeyDown, bool, (int KeyCode), return input::IsKeyDown(input::Key(KeyCode)))

// UI
STRUCT_MEMBER(CreateUICanvas, void*, (const char* Name, const char* Source, engine::plugin::PluginCanvasInterface * Canvas), \
{ auto c = engine::UICanvas::CreateNew<PluginUICanvas>(); if (c) { c->LoadElement(Name, Source, Canvas); } return c; })

STRUCT_MEMBER(CreateUIBox, kuiUIBox*, (const char* Name, void* UICanvas), \
{ return (kuiUIBox*)((PluginUICanvas*)UICanvas)->CreateElement(Name); })

STRUCT_MEMBER(DeleteUIBox, void, (kuiUIBox * Target), delete (kui::UIBox*)Target)

STRUCT_MEMBER(GetDynamicChild, kuiUIBox*, (kuiUIBox * Target, const char* Name), \
{ auto c = (kui::markup::UIDynMarkupBox*)Target; return (kuiUIBox*)c->NamedChildren[Name]; })
STRUCT_MEMBER(GetCanvasChild, kuiUIBox*, (void* UICanvas, const char* Name), \
{ auto c = (PluginUICanvas*)UICanvas; return (kuiUIBox*)c->GetElement(Name); })
STRUCT_MEMBER(AddChild, void, (kuiUIBox * Parent, kuiUIBox * Child), \
{ ((kui::UIBox*)Parent)->AddChild((kui::UIBox*)Child); })

STRUCT_MEMBER(UITextSetText, void, (kuiUIBox * UIElement, const char* Text), \
{ auto c = (kui::UIText*)UIElement; c->SetText(Text); })

STRUCT_MEMBER(UITextFieldEdit, void, (kuiUIBox * UIElement), \
{ auto c = (kui::UITextField*)UIElement; c->Edit(); })

STRUCT_MEMBER(UITextFieldGetText, const char*, (kuiUIBox * UIElement), \
{ auto c = (kui::UITextField*)UIElement; return c->GetText().c_str(); })

STRUCT_MEMBER(UITextFieldSetText, void, (kuiUIBox * UIElement, const char* Text), \
{ auto c = (kui::UITextField*)UIElement; c->SetText(Text); })

STRUCT_MEMBER(UITextFieldOnChanged, void, (kuiUIBox * UIElement, CallbackFn Callback, void* UserData), \
{ auto c = (kui::UITextField*)UIElement; c->OnChanged = ([Callback, UserData]() {Callback(UserData); }); })

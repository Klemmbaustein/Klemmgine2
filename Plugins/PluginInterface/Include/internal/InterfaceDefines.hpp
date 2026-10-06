/// @cond

// Cursed preprocessor logic for the plugin interface struct.
// I am sorry.

// Core
STRUCT_MEMBER(Log, void, (const char* Message), Log::Info(Message))

// Editor
STRUCT_MEMBER_CALL_DIRECT(IsEditorActive, bool, (), editor::IsActive)
STRUCT_MEMBER(GameHasFocus, bool, (), return Engine::GameHasFocus)

// Console

STRUCT_MEMBER(ConsoleExecuteCommand, void, (const char* cmd), console::ExecuteCommand(cmd); )

// Objects
STRUCT_MEMBER(RegisterObj, int32, (const char* Name, SceneObject* (*Func)(void* UserData), void* UserData, const char* Category), \
return Reflection::RegisterObject(Name, [Func, UserData]() -> SceneObject* { return Func(UserData); }, 0, Category);)

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
STRUCT_MEMBER(InputIsKeyDown, bool, (int KeyCode), return input::IsKeyHeld(input::Key(KeyCode)))
STRUCT_MEMBER(InputIsKeyPressed, bool, (int KeyCode), return input::IsKeyPressed(input::Key(KeyCode)))

// UI

STRUCT_MEMBER(GetUIContext, kui::UIContext*, (), \
{ return kui::UIContext::Get(); })

STRUCT_MEMBER(GetMainWindow, kui::Window*, (), \
{ return kui::Window::GetActiveWindow(); })

STRUCT_MEMBER(CreateUICanvas, void*, (engine::plugin::PluginCanvasInterface * Canvas), \
{ auto c = engine::UICanvas::CreateNew<PluginUICanvas>(); if (c) { c->LoadPluginCanvas(Canvas); } return c; })

STRUCT_MEMBER_CALL_DIRECT(GetLogSize, size_t, (), \
	Log::GetLogMessagesCount)

STRUCT_MEMBER(GetLogMessages, engine::plugin::LogEntry*, (size_t* OutSize), \
{ return ::GetLog(OutSize); })
/// @endcond
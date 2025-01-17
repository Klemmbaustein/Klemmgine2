STRUCT_MEMBER(RegisterObj, int32, (std::string Name, SceneObject* (*Func)(void* UserData), void* UserData), \
	return Reflection::RegisterObject(Name, [Func, UserData]() -> SceneObject* { return Func(UserData); }))

STRUCT_MEMBER(Log, void, (const char* Message), Log::Info(Message))
STRUCT_MEMBER(CreateObj, engine::SceneObject*, (engine::Scene* Scn, int32 TypeID, Vector3 pos, Rotation3 rot, Vector3 scl), \
	if (!Scn) Scn = Scene::GetMain(); \
		return Scn->CreateObjectFromID(TypeID, pos, rot, scl))

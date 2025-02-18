STRUCT_MEMBER(RegisterObj, int32, (std::string Name, SceneObject* (*Func)(void* UserData), void* UserData), \
	return Reflection::RegisterObject(Name, [Func, UserData]() -> SceneObject* { return Func(UserData); }))

	STRUCT_MEMBER(Log, void, (const char* Message), Log::Info(Message))
	STRUCT_MEMBER(CreateObj, engine::SceneObject*, (engine::Scene * Scn, int32 TypeID, Vector3 pos, Rotation3 rot, Vector3 scl), \
		if (!Scn) Scn = Scene::GetMain(); \
			return Scn->CreateObjectFromID(TypeID, pos, rot, scl))
	STRUCT_MEMBER(GetObjName, const char*, (engine::SceneObject * Target), return Target->Name.c_str())
STRUCT_MEMBER(NewMeshComponent, void*, (), return new MeshComponent())
STRUCT_MEMBER(ObjectAttachComponent, void, (void* Obj, void* Comp), ((engine::SceneObject*)Obj)->Attach((engine::ObjectComponent*)Comp))
STRUCT_MEMBER(ComponentAttach, void, (void* Parent, void* Comp), ((engine::ObjectComponent*)Parent)->Attach((engine::ObjectComponent*)Comp))
STRUCT_MEMBER(MeshComponentLoad, void, (void* Comp, const char* Name), ((engine::MeshComponent*)Comp)->Load(AssetRef::Convert(Name)))

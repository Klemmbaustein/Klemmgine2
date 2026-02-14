#include "JoltPhysics.h"
#include <Engine/Physics/Physics.h>
#include <Core/Error/StackTrace.h>
#include <Core/Log.h>
#include <cstdarg>
#include <Core/Error/EngineAssert.h>
#include <Engine/Stats.h>
#include <mutex>
#include <algorithm>
#include <Core/ThreadPool.h>
#include <Engine/MainThread.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/PhysicsSettings.h>

using namespace engine;
using namespace engine::physics;

static std::mutex LoadedModelsMutex;

static bool LayerMask(Layer Bit, Layer Mask)
{
	return bool(Bit & Mask);
}

inline static JPH::Vec3 ToJPHVec3(const Vector3& Vec)
{
	return JPH::Vec3(Vec.X, Vec.Y, Vec.Z);
}

inline static JPH::Vec4 ToJPHVec4(const glm::vec4& Vec)
{
	return JPH::Vec4(Vec.x, Vec.y, Vec.z, Vec.w);
}

inline static JPH::Quat ToJPHQuat(Rotation3 Rot)
{
	Rot = Rot.EulerVector(true);
	return JPH::Quat::sEulerAngles(JPH::Vec3(Rot.P, Rot.Y, Rot.R));
}

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	static bool CheckForStaticDynamic(Layer inObject1, Layer inObject2)
	{
		if (LayerMask(inObject1, Layer::Static) && LayerMask(inObject2, Layer::Dynamic))
		{
			return true;
		}

		return LayerMask(inObject1, inObject2);
	}

	static bool ShouldLayersCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
	{
		if (LayerMask((Layer)inObject1, Layer::Trigger))
		{
			return true;
		}

		if (CheckForStaticDynamic((Layer)inObject1, (Layer)inObject2)) return true;
		if (CheckForStaticDynamic((Layer)inObject2, (Layer)inObject1)) return true;
		return inObject1 & inObject2;
	}

	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		return ShouldLayersCollide(inObject1, inObject2);
	}
};

class JoltJobSystemImpl final : public JPH::JobSystemWithBarrier
{
public:

	ThreadPool* Pool = nullptr;

	std::map<Job*, JobFunction> Jobs;

	JoltJobSystemImpl(ThreadPool* Pool)
	{
		Init(JPH::cMaxPhysicsBarriers);
		this->Pool = Pool;
	}

	int GetMaxConcurrency() const override
	{
		return ThreadPool::Main()->NumJobs();
	}

	JobHandle CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, uint32 inNumDependencies = 0) override
	{
		auto j = new Job(inName, inColor, this, inJobFunction, inNumDependencies);
		Jobs.emplace(j, inJobFunction);

		if (inNumDependencies == 0)
			QueueJob(j);

		return JobHandle(j);
	}

	void QueueJob(Job* inJob)
	{
		Pool->AddJob(std::bind(&Job::Execute, inJob));
	}

	void QueueJobs(Job** inJobs, JPH::uint inNumJobs)
	{
		for (JPH::uint i = 0; i < inNumJobs; i++)
		{
			QueueJob(inJobs[i]);
		}
	}

	void FreeJob(Job* inJob)
	{
		Jobs.erase(inJob);
	}
};

namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer STATIC(0);
	static constexpr JPH::BroadPhaseLayer DYNAMIC(1);
	static constexpr JPH::BroadPhaseLayer CUSTOM(2);
	static constexpr JPH::uint NUM_LAYERS = 3;
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		Layer EngineLayer = (Layer)inLayer;
		if (bool(EngineLayer & Layer::Static))
		{
			return BroadPhaseLayers::STATIC;
		}
		if (bool(EngineLayer & Layer::Dynamic))
		{
			return BroadPhaseLayers::DYNAMIC;
		}
		return BroadPhaseLayers::CUSTOM;
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch (inLayer.GetValue())
		{
		case JPH::BroadPhaseLayer::Type(BroadPhaseLayers::STATIC):
			return "Static";
		case JPH::BroadPhaseLayer::Type(BroadPhaseLayers::DYNAMIC):
			return "Dynamic";
		case JPH::BroadPhaseLayer::Type(BroadPhaseLayers::CUSTOM):
			return "Custom";
		default:
			break;
		}
		return "Unkown";
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		Layer EngineLayer = (Layer)inLayer1;
		if (bool(EngineLayer & Layer::Dynamic))
		{
			return inLayer2 == BroadPhaseLayers::STATIC;
		}
		if (bool(EngineLayer & Layer::Static))
		{
			return inLayer2 == BroadPhaseLayers::STATIC || inLayer2 == BroadPhaseLayers::DYNAMIC;
		}
		return inLayer2 == BroadPhaseLayers::CUSTOM;
	}
};

namespace engine::internal
{
	BPLayerInterfaceImpl BroadPhaseLayers;
	ObjectVsBroadPhaseLayerFilterImpl ObjectVsBroadPhaseFilter;
	ObjectLayerPairFilterImpl ObjectVsObjectFilter;
}

static void JoltPhysicsTrace(const char* inFMT, ...)
{
	va_list list;
	va_start(list, inFMT);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Log::Error(buffer);

	Log::Error(error::GetStackTrace());
}

bool internal::JoltInstance::IsInitialized = false;
JPH::TempAllocatorImpl* internal::JoltInstance::TempAllocator = nullptr;
JoltJobSystemImpl* internal::JoltInstance::JobSystem = nullptr;

void engine::internal::JoltInstance::InitJolt()
{
	if (IsInitialized)
		return;

	JPH::RegisterDefaultAllocator();

	JPH::Trace = &JoltPhysicsTrace;
	//JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;);

	// Create a factory
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	TempAllocator = new JPH::TempAllocatorImpl(1024 * 1024);
	JobSystem = new JoltJobSystemImpl(ThreadPool::Main());

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	IsInitialized = true;
}

JPH::MeshShape* engine::internal::JoltInstance::CreateNewMeshShape(GraphicsModel* From, bool ExtraReference)
{
	JPH::VertexList Vertices;
	JPH::IndexedTriangleList Indices;

	auto& Meshes = From->Data->Meshes;

	uint32 IndicesPosition = 0;
	for (auto& m : Meshes)
	{
		for (auto& i : m.Vertices)
		{
			Vertices.push_back(JPH::Float3(i.Position.X, i.Position.Y, i.Position.Z));
		}

		for (size_t i = 0; i < m.Indices.size(); i += 3)
		{
			JPH::IndexedTriangle Tri = JPH::IndexedTriangle(
				m.Indices[i] + IndicesPosition,
				m.Indices[i + 1] + IndicesPosition,
				m.Indices[i + 2] + IndicesPosition
			);
			Indices.push_back(Tri);
		}
		IndicesPosition += uint32(m.Vertices.size());
	}

	JPH::MeshShapeSettings Settings = JPH::MeshShapeSettings(Vertices, Indices);
	JPH::ShapeSettings::ShapeResult MeshResult;
	JPH::MeshShape* Shape = new JPH::MeshShape(Settings, MeshResult);
	if (!MeshResult.IsValid())
	{
		Log::Error("Error creating collision shape: " + std::string(MeshResult.GetError()));
		return nullptr;
	}

	auto BodyModel = From;
	// Unload the Jolt Mesh Shape when the model data is unloaded.
	// It will also be unloaded when the physics body itself is destroyed.
	BodyModel->OnDereferenced.Add(this, [this, BodyModel]() {
		UnloadMesh(BodyModel);
	});

	Shape->AddRef();

	LoadedMeshes.insert({ BodyModel, PhysicsMesh{
		.Shape = Shape,
		// One reference for the body, one for the mesh itself.
		.References = uint64(ExtraReference ? 2 : 1),
		} });
	return Shape;
}

void engine::internal::JoltInstance::UnloadMesh(GraphicsModel* Mesh)
{
	auto Found = LoadedMeshes.find(Mesh);
	if (Found != LoadedMeshes.end())
	{
		Found->second.References--;
		if (Found->second.References == 0)
		{
			Found->second.Shape->Release();
			LoadedMeshes.erase(Found);
		}
	}
}

JPH::BodyCreationSettings engine::internal::JoltInstance::CreateJoltShapeFromBody(PhysicsBody* Body)
{
	Vector3 Position, Scale;
	Rotation3 Rotation;
	Body->BodyTransform.Decompose(Position, Rotation, Scale);

	switch (Body->Type)
	{
	case PhysicsBody::BodyType::Sphere:
	{
		SphereBody* SpherePtr = static_cast<SphereBody*>(Body);
		return JPH::BodyCreationSettings(new JPH::SphereShape(Scale.X),
			ToJPHVec3(Position),
			ToJPHQuat(Rotation),
			JPH::EMotionType(Body->ColliderMovability),
			JPH::ObjectLayer(Body->CollisionLayers));
	}
	case PhysicsBody::BodyType::Box:
	{
		BoxBody* BoxPtr = static_cast<BoxBody*>(Body);
		return JPH::BodyCreationSettings(new JPH::BoxShape(ToJPHVec3(Scale)),
			ToJPHVec3(Position),
			ToJPHQuat(Rotation),
			JPH::EMotionType(Body->ColliderMovability),
			JPH::ObjectLayer(Body->CollisionLayers));
	}
	case PhysicsBody::BodyType::Capsule:
	{
		CapsuleBody* CapsulePtr = static_cast<CapsuleBody*>(Body);
		JPH::CapsuleShapeSettings Settings = JPH::CapsuleShapeSettings(Scale.Y, Scale.X);
		JPH::Shape::ShapeResult r;
		return JPH::BodyCreationSettings(new JPH::CapsuleShape(Settings, r),
			ToJPHVec3(Position),
			ToJPHQuat(Rotation),
			JPH::EMotionType(Body->ColliderMovability),
			JPH::ObjectLayer(Body->CollisionLayers));
	}
	case PhysicsBody::BodyType::Mesh:
	{
		MeshBody* MeshPtr = static_cast<MeshBody*>(Body);

		std::lock_guard g{ LoadedModelsMutex };

		JPH::MeshShape* UsedShape = nullptr;

		auto Found = LoadedMeshes.find(MeshPtr->Model);
		// Don't create duplicate mesh shapes, reference existing ones instead.
		if (Found != LoadedMeshes.end())
		{
			Found->second.References++;
			UsedShape = Found->second.Shape;
		}
		else
		{
			UsedShape = CreateNewMeshShape(MeshPtr->Model);
		}

		JPH::ScaledShapeSettings Settings = JPH::ScaledShapeSettings(UsedShape,
			ToJPHVec3(Scale != 0 ? Scale : 0.001f));
		JPH::ShapeSettings::ShapeResult r;
		JPH::ScaledShape* Shape = new JPH::ScaledShape(Settings, r);

		if (r.HasError())
		{
			Log::Error(string(r.GetError()));
		}

		return JPH::BodyCreationSettings(Shape,
			ToJPHVec3(Position),
			ToJPHQuat(Rotation),
			JPH::EMotionType::Static,
			JPH::ObjectLayer(MeshPtr->CollisionLayers));
	}
	case PhysicsBody::BodyType::HeightMap:
	{
		HeightMapBody* MapPtr = static_cast<HeightMapBody*>(Body);

		std::lock_guard g{ LoadedModelsMutex };

		JPH::HeightFieldShapeSettings Settings = JPH::HeightFieldShapeSettings(MapPtr->Samples.data(),
			ToJPHVec3(Position), ToJPHVec3(Scale), MapPtr->Size);
		JPH::ShapeSettings::ShapeResult r;
		JPH::HeightFieldShape* Shape = new JPH::HeightFieldShape(Settings, r);

		if (r.HasError())
		{
			Log::Error(string(r.GetError()));
		}

		return JPH::BodyCreationSettings(Shape,
			JPH::Vec3(0, 0, 0),
			ToJPHQuat(Rotation),
			JPH::EMotionType::Static,
			JPH::ObjectLayer(MapPtr->CollisionLayers));
	}
	}
	ENGINE_UNREACHABLE();
	return JPH::BodyCreationSettings();
}

engine::internal::JoltInstance::JoltInstance()
{
	const uint32 cMaxBodies = 65536;

	const uint32 cNumBodyMutexes = 0;
	const uint32 cMaxBodyPairs = 10240;
	const uint32 cMaxContactConstraints = 1024;

	System = new JPH::PhysicsSystem();
	System->Init(cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		BroadPhaseLayers,
		ObjectVsBroadPhaseFilter,
		ObjectVsObjectFilter);

	JoltBodyInterface = &System->GetBodyInterface();
}

engine::internal::JoltInstance::~JoltInstance()
{
	std::unordered_map MeshCopy = LoadedMeshes;
	for (auto& i : MeshCopy)
	{
		if (i.first->OnDereferenced.IsListener(this))
			i.first->OnDereferenced.Remove(this);
		UnloadMesh(i.first);
	}

	delete System;
}

void engine::internal::JoltInstance::Update()
{
	// Loading models while the system is updating is a bad idea (don't ask how I know)
	std::lock_guard g{ LoadedModelsMutex };

	// In case time scale is less than 0 (support this for fun)
	System->Update(std::clamp<float>(stats::DeltaTime, 0, 0.1f), 1, TempAllocator, JobSystem);
}

void engine::internal::JoltInstance::AddBody(PhysicsBody* Body, bool StartActive, bool StartCollisionEnabled)
{
	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
		if (!Body->ShapeInfo)
		{
			return;
		}
	}
	auto JoltShape = static_cast<JPH::BodyCreationSettings*>(Body->ShapeInfo);
	PhysicsBodyInfo info;
	info.Body = Body;

	if (StartCollisionEnabled)
	{
		info.ID = JoltBodyInterface->CreateAndAddBody(*JoltShape, StartActive
			? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
	}
	else
	{
		info.ID = JoltBodyInterface->CreateBody(*JoltShape)->GetID();
	}

	Body->PhysicsSystemBody = &Bodies.emplace(info.ID, info).first->second;
	JoltBodyInterface->SetUserData(info.ID, reinterpret_cast<uint64>(Body->PhysicsSystemBody));
	Body->IsActive = StartActive;
	Body->IsCollisionEnabled = StartCollisionEnabled;
}

void engine::internal::JoltInstance::RemoveBody(engine::physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	if (Body->Type == PhysicsBody::BodyType::Mesh)
	{
		MeshBody* MeshPtr = static_cast<MeshBody*>(Body);
		UnloadMesh(MeshPtr->Model);
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	if (Body->IsCollisionEnabled)
	{
		JoltBodyInterface->RemoveBody(Info->ID);
		JoltBodyInterface->DestroyBody(Info->ID);
	}
	Bodies.erase(Info->ID);
	Body->PhysicsSystemBody = nullptr;
	auto JoltShape = static_cast<JPH::BodyCreationSettings*>(Body->ShapeInfo);
	delete JoltShape;
	Body->ShapeInfo = nullptr;
}

void engine::internal::JoltInstance::CreateShape(engine::physics::PhysicsBody* Body)
{
	Body->ShapeInfo = new JPH::BodyCreationSettings(CreateJoltShapeFromBody(Body));
}

void engine::internal::JoltInstance::SetBodyPosition(engine::physics::PhysicsBody* Body, Vector3 NewPosition)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetPosition(Info->ID, ToJPHVec3(NewPosition), JPH::EActivation::Activate);
}

Vector3 engine::internal::JoltInstance::GetBodyPosition(physics::PhysicsBody* Body) const
{
	if (!Body->PhysicsSystemBody)
	{
		return 0;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	auto Pos = JoltBodyInterface->GetPosition(Info->ID);
	return Vector3(Pos.GetX(), Pos.GetY(), Pos.GetZ());
}

std::pair<Vector3, Rotation3> engine::internal::JoltInstance::GetBodyPositionAndRotation(physics::PhysicsBody* Body) const
{
	if (!Body->PhysicsSystemBody)
	{
		return {};
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JPH::Vec3 Pos;
	JPH::Quat Rot;
	JoltBodyInterface->GetPositionAndRotation(Info->ID, Pos, Rot);
	JPH::Vec3 RotEuler = Rot.GetEulerAngles();
	return { Vector3(Pos.GetX(), Pos.GetY(), Pos.GetZ()),
		Rotation3(RotEuler.GetX(), RotEuler.GetY(), RotEuler.GetZ(), true) };
}

void engine::internal::JoltInstance::SetBodyRotation(engine::physics::PhysicsBody* Body, Vector3 NewRotation)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetRotation(Info->ID, ToJPHQuat(NewRotation), JPH::EActivation::Activate);
}

void engine::internal::JoltInstance::SetBodyPositionAndRotation(engine::physics::PhysicsBody* Body,
	Vector3 NewPosition, Rotation3 NewRotation)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetPositionAndRotation(Info->ID, ToJPHVec3(NewPosition),
		ToJPHQuat(NewRotation), JPH::EActivation::Activate);
}

void engine::internal::JoltInstance::ScaleBody(engine::physics::PhysicsBody* Body, Vector3 ScaleFactor) const
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);

	auto s = JoltBodyInterface->GetShape(Info->ID);

	auto t = s->GetSubType();

	if (t == JPH::EShapeSubType::Scaled)
	{
		auto scaled = static_cast<const JPH::ScaledShape*>(s.GetPtr());
		s = scaled->GetInnerShape();
		ScaleFactor *= Vector3(scaled->GetScale().GetX(), scaled->GetScale().GetY(), scaled->GetScale().GetZ());
	}
	auto ShapeInfo = s->ScaleShape(ToJPHVec3(ScaleFactor));
	JoltBodyInterface->SetShape(Info->ID, ShapeInfo.Get(), true, JPH::EActivation::Activate);
}

void engine::internal::JoltInstance::SetBodyActive(engine::physics::PhysicsBody* Body, bool IsActive)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	if (IsActive)
		JoltBodyInterface->ActivateBody(Info->ID);
	else
		JoltBodyInterface->DeactivateBody(Info->ID);
	Body->IsActive = IsActive;
}

void engine::internal::JoltInstance::SetBodyCollisionEnabled(engine::physics::PhysicsBody* Body,
	bool IsCollisionEnabled)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	if (IsCollisionEnabled)
	{
		JoltBodyInterface->AddBody(Info->ID, Body->IsActive
			? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
	}
	else
	{
		JoltBodyInterface->RemoveBody(Info->ID);
	}
	Body->IsCollisionEnabled = IsCollisionEnabled;
}

void engine::internal::JoltInstance::PreLoadMesh(GraphicsModel* Mesh)
{
	std::lock_guard g{ LoadedModelsMutex };

	if (LoadedMeshes.contains(Mesh))
		return;

	CreateNewMeshShape(Mesh);
	UnloadMesh(Mesh);
}

class ObjectLayerFilterImpl : public JPH::ObjectLayerFilter
{
public:
	Layer ObjLayer = Layer::Static;
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override
	{
		return ObjectLayerPairFilterImpl::ShouldLayersCollide(JPH::ObjectLayer(ObjLayer), inLayer);
	}
};

class CastShapeCollectorImpl : public JPH::CastShapeCollector
{
public:
	CastShapeCollectorImpl()
	{

	}

	std::vector<HitResult> Hits;
	internal::JoltInstance* Instance = nullptr;

	virtual void AddHit(const ResultType& inResult) override
	{
		HitResult r;
		r.Hit = true;
		r.Depth = inResult.mPenetrationDepth;
		r.Normal = -Vector3(inResult.mPenetrationAxis.GetX(),
			inResult.mPenetrationAxis.GetY(), inResult.mPenetrationAxis.GetZ()).Normalize();
		r.ImpactPoint = Vector3(inResult.mContactPointOn2.GetX(),
			inResult.mContactPointOn2.GetY(), inResult.mContactPointOn2.GetZ());
		r.Distance = inResult.mFraction;

		PhysicsBody* BodyInfo = Instance->Bodies[inResult.mBodyID2].Body;
		r.HitComponent = BodyInfo->Parent;
		Hits.push_back(r);
	}
};

class BodyFilterImpl : public JPH::BodyFilter
{
public:
	std::set<SceneObject*>* ObjectsToIgnore = nullptr;
	internal::JoltInstance* Instance = nullptr;
	JPH::BodyID ThisBody;

	virtual bool ShouldCollide(const JPH::BodyID& inObject) const override
	{
		if (ThisBody == inObject)
		{
			return false;
		}

		if (ObjectsToIgnore->empty())
			return true;
		PhysicsBody* BodyInfo = Instance->Bodies[inObject].Body;
		//auto val = Instance->JoltBodyInterface->GetUserData(inObject);
		//PhysicsBody* BodyInfo = reinterpret_cast<PhysicsBody*>(val);
		return ObjectsToIgnore->find(BodyInfo->Parent->RootObject) == ObjectsToIgnore->end();
	}
};

class CollisionShapeCollectorImpl : public JPH::CollideShapeCollector
{
public:

	CollisionShapeCollectorImpl()
	{

	}

	std::vector<HitResult> Hits;
	internal::JoltInstance* Instance = nullptr;

	virtual void AddHit(const ResultType& inResult) override
	{
		HitResult r;
		r.Hit = true;
		r.Depth = inResult.mPenetrationDepth;
		r.Normal = -Vector3(inResult.mPenetrationAxis.GetX(),
			inResult.mPenetrationAxis.GetY(), inResult.mPenetrationAxis.GetZ()).Normalize();
		r.ImpactPoint = Vector3(inResult.mContactPointOn2.GetX(),
			inResult.mContactPointOn2.GetY(), inResult.mContactPointOn2.GetZ());

		PhysicsBody* BodyInfo = Instance->Bodies[inResult.mBodyID2].Body;
		r.HitComponent = BodyInfo->Parent;
		Hits.push_back(r);
	};
};

std::vector<physics::HitResult> engine::internal::JoltInstance::CollisionTest(Transform At,
	physics::PhysicsBody* Body, physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
		if (!Body->ShapeInfo)
		{
			return {};
		}
	}
	auto JoltShape = static_cast<JPH::BodyCreationSettings*>(Body->ShapeInfo);

	glm::mat4 mat = At.Matrix;

	Vector3 Position, Scale;
	Rotation3 Rotation;
	At.Decompose(Position, Rotation, Scale);

	JPH::Mat44 ResultMat = JPH::Mat44(ToJPHVec4(mat[0]), ToJPHVec4(mat[1]), ToJPHVec4(mat[2]), ToJPHVec4(mat[3]));

	CollisionShapeCollectorImpl cl;
	cl.Instance = this;
	JPH::CollideShapeSettings Settings = JPH::CollideShapeSettings();

	JPH::BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;
	ObjF.Instance = this;

	System->GetNarrowPhaseQuery().CollideShape(JoltShape->GetShape(),
		JPH::Vec3(1, 1, 1), ResultMat, Settings, JPH::Vec3(), cl, BplF, LayerF, ObjF);

	return cl.Hits;
}

std::vector<HitResult> internal::JoltInstance::ShapeCastBody(
	physics::PhysicsBody* Body,
	Transform StartPos, Vector3 EndPos,
	physics::Layer Layers,
	std::set<SceneObject*> ObjectsToIgnore)
{
	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
		if (!Body->ShapeInfo)
		{
			return {};
		}
	}

	auto JoltShape = static_cast<JPH::BodyCreationSettings*>(Body->ShapeInfo);

	glm::mat4 mat = StartPos.Matrix;

	Vector3 Position, Scale;
	Rotation3 Rotation;
	StartPos.Decompose(Position, Rotation, Scale);

	Vector3 Direction = EndPos - Position;

	JPH::Mat44 ResultMat = JPH::Mat44(ToJPHVec4(mat[0]), ToJPHVec4(mat[1]), ToJPHVec4(mat[2]), ToJPHVec4(mat[3]));

	CastShapeCollectorImpl cl;
	cl.Instance = this;
	JPH::ShapeCastSettings s;

	JPH::BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.Instance = this;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;

	JPH::RShapeCast c = JPH::RShapeCast(JoltShape->GetShape(), ToJPHVec3(1), ResultMat, ToJPHVec3(Direction));
	System->GetNarrowPhaseQuery().CastShape(c, s, JPH::Vec3(0, 0, 0), cl, BplF, LayerF, ObjF);
	return cl.Hits;
}

engine::physics::HitResult engine::internal::JoltInstance::LineCast(
	Vector3 Start, Vector3 End,
	engine::physics::Layer Layers,
	std::set<SceneObject*> ObjectsToIgnore)
{
	JPH::RRayCast Cast = JPH::RRayCast(ToJPHVec3(Start), ToJPHVec3(End - Start));
	JPH::RayCastResult HitInfo;

	JPH::BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.Instance = this;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;
	bool Hit = System->GetNarrowPhaseQuery().CastRay(Cast, HitInfo, BplF, LayerF, ObjF);

	HitResult r;
	r.Hit = Hit;
	if (Hit)
	{
		auto val = Bodies.find(HitInfo.mBodyID);
		if (val != Bodies.end())
		{
			PhysicsBodyInfo BodyInfo = (*val).second;
			r.HitComponent = BodyInfo.Body->Parent;
			JPH::Vec3 ImpactPos = Cast.GetPointOnRay(HitInfo.mFraction);
			r.ImpactPoint = Vector3(ImpactPos.GetX(), ImpactPos.GetY(), ImpactPos.GetZ());

			JPH::Vec3 Normal = JoltBodyInterface->GetTransformedShape(BodyInfo.ID)
				.GetWorldSpaceSurfaceNormal(HitInfo.mSubShapeID2, ImpactPos);
			r.Distance = HitInfo.mFraction;
			r.Normal = Vector3(Normal.GetX(), Normal.GetY(), Normal.GetZ());
		}
	}
	return r;
}

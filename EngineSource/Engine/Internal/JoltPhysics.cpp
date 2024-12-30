#include "JoltPhysics.h"
#include <Engine/Physics/Physics.h>
#include <Engine/Error/StackTrace.h>
#include <Engine/Log.h>
#include <cstdarg>
#include <Engine/Error/EngineAssert.h>
#include <Engine/Stats.h>
using namespace engine;
using namespace engine::physics;

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
		if (CheckForStaticDynamic((Layer)inObject1, (Layer)inObject2)) return true;
		if (CheckForStaticDynamic((Layer)inObject2, (Layer)inObject1)) return true;
		return inObject1 & inObject2;
	}

	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		return ShouldLayersCollide(inObject1, inObject2);
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

namespace engine::internal::physics
{
	BPLayerInterfaceImpl BroadPhaseLayers;
	ObjectVsBroadPhaseLayerFilterImpl ObjectVsBroadPhaseFilter;
	ObjectLayerPairFilterImpl ObjectVsObjectFilter;
}

// Callback for traces, connect this to your own trace function if you have one
static void JoltPhysicsTrace(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Log::Error(buffer);

	Log::Error(error::GetStackTrace());
}

void engine::internal::JoltInstance::InitJolt()
{
	JPH::RegisterDefaultAllocator();

	JPH::Trace = &JoltPhysicsTrace;
	//JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;);

	// Create a factory
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	auto* TempAllocator = new JPH::TempAllocatorImpl(1024 * 1024);
	auto* JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.

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
			(JPH::ObjectLayer)Body->CollisionLayers);
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
		JPH::CapsuleShapeSettings Settings = JPH::CapsuleShapeSettings(Scale.X, Scale.Y);
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

		auto Found = LoadedMeshes.find(MeshPtr->Model);

		if (Found != LoadedMeshes.end())
		{
			Found->second.References++;
			return JPH::BodyCreationSettings(Found->second.Shape,
				ToJPHVec3(Position),
				ToJPHQuat(Rotation),
				JPH::EMotionType::Static,
				JPH::ObjectLayer(MeshPtr->CollisionLayers));
		}

		JPH::VertexList Vertices;
		JPH::IndexedTriangleList Indices;

		auto& Meshes = MeshPtr->Model->Data->Meshes;

		uint32 IndicesPosition = 0;
		for (auto& m : Meshes)
		{
			for (auto& i : m.Vertices)
			{
				Vertices.push_back(JPH::Float3(i.Position.X * Scale.X, i.Position.Y * Scale.Y, i.Position.Z * Scale.Z));
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
			return JPH::BodyCreationSettings();
		}

		LoadedMeshes.insert({ MeshPtr->Model, PhysicsMesh{
			.Shape = Shape,
			.References = 1,
			} });

		return JPH::BodyCreationSettings(Shape,
			ToJPHVec3(Position),
			ToJPHQuat(Rotation),
			JPH::EMotionType::Static,
			JPH::ObjectLayer(MeshPtr->CollisionLayers));
	}
	}
	ENGINE_UNREACHABLE();
	return JPH::BodyCreationSettings();
}

engine::internal::JoltInstance::JoltInstance()
{
	using namespace engine::internal::physics;

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

	System->SetGravity(System->GetGravity() * 4);

	JoltBodyInterface = &System->GetBodyInterface();
}

void engine::internal::JoltInstance::Update()
{
	System->Update(stats::DeltaTime, 1, TempAllocator, JobSystem);
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
		JPH::BodyID ID = JoltBodyInterface->CreateAndAddBody(*JoltShape, StartActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
		info.ID = ID;
	}
	else
	{
		JoltBodyInterface->CreateBodyWithID(info.ID, *JoltShape);
	}
	Body->PhysicsSystemBody = &Bodies.emplace(info.ID, info).first->second;
	JoltBodyInterface->SetUserData(info.ID, uint64(Body->PhysicsSystemBody));
	Body->IsActive = StartActive;
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
		auto Found = LoadedMeshes.find(MeshPtr->Model);
		if (Found != LoadedMeshes.end())
		{
			Found->second.References--;
			if (Found->second.References == 0)
			{
				Log::Info("Unload mesh");
				LoadedMeshes.erase(Found);
			}
		}
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	if (Body->IsCollisionEnabled)
		JoltBodyInterface->RemoveBody(Info->ID);
	JoltBodyInterface->DestroyBody(Info->ID);
	Bodies.erase(Info->ID);
	Body->PhysicsSystemBody = nullptr;
	delete Body->ShapeInfo;
	Body->ShapeInfo = nullptr;
	Info->ID = JPH::BodyID();
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

void engine::internal::JoltInstance::SetBodyRotation(engine::physics::PhysicsBody* Body, Vector3 NewRotation)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetRotation(Info->ID, ToJPHQuat(NewRotation), JPH::EActivation::Activate);
}

void engine::internal::JoltInstance::SetBodyPositionAndRotation(engine::physics::PhysicsBody* Body, Vector3 NewPosition, Rotation3 NewRotation)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetPositionAndRotation(Info->ID, ToJPHVec3(NewPosition), ToJPHQuat(NewRotation), JPH::EActivation::Activate);
}

void engine::internal::JoltInstance::ScaleBody(engine::physics::PhysicsBody* Body, Vector3 ScaleFactor)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	auto ShapeInfo = JoltBodyInterface->GetShape(Info->ID).GetPtr()->ScaleShape(ToJPHVec3(ScaleFactor));
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

void engine::internal::JoltInstance::SetBodyCollisionEnabled(engine::physics::PhysicsBody* Body, bool IsCollisionEnabled)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	if (IsCollisionEnabled)
	{
		JoltBodyInterface->AddBody(Info->ID, Body->IsActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
	}
	else
	{
		JoltBodyInterface->RemoveBody(Info->ID);
	}
	Body->IsCollisionEnabled = IsCollisionEnabled;
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

class BodyFilterImpl : public JPH::BodyFilter
{
public:
	std::set<SceneObject*>* ObjectsToIgnore = nullptr;
	internal::JoltInstance* Instance = nullptr;

	virtual bool ShouldCollide(const JPH::BodyID& inObject) const override
	{
		auto body = Instance->Bodies.find(inObject);
		if (body != Instance->Bodies.end()
			&& body->second.Body->Parent
			&& body->second.Body->Parent->RootObject)
		{
			return ObjectsToIgnore->find(body->second.Body->Parent->RootObject) == ObjectsToIgnore->end();
		}
		return true;
	}
};

engine::physics::HitResult engine::internal::JoltInstance::LineCast(Vector3 Start, Vector3 End, engine::physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
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

			JPH::Vec3 Normal = JoltBodyInterface->GetTransformedShape(BodyInfo.ID).GetWorldSpaceSurfaceNormal(HitInfo.mSubShapeID2, ImpactPos);
			r.Distance = HitInfo.mFraction;
			r.Normal = Vector3(Normal.GetX(), Normal.GetY(), Normal.GetZ());
		}
	}
	return r;
}

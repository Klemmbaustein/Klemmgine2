#pragma once
#include <Core/Platform/CodeAnalysis.h>
#include <Engine/Physics/Physics.h>
#include <Core/Log.h>
#include <Engine/Objects/SceneObject.h>
CODE_ANALYSIS_BEGIN_EXTERNAL_HEADER
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/Body.h>
CODE_ANALYSIS_END_EXTERNAL_HEADER

namespace engine::internal
{
	class EngineContactListenerImpl : JPH::ContactListener
	{
	public:
		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
			const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
		{
			auto HitBody1 = reinterpret_cast<physics::PhysicsBody*>(inBody1.GetUserData());
			auto HitBody2 = reinterpret_cast<physics::PhysicsBody*>(inBody1.GetUserData());

			Log::Info(str::Format("Collision - %s, %s", HitBody1->Parent->GetRootObject()->Name.c_str(), HitBody2->Parent->GetRootObject()->Name.c_str()));
		}
	};
}
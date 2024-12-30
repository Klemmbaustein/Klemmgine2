#pragma once
#include <Engine/Vector.h>
#include <Engine/Transform.h>
#include <Engine/Objects/Components/ObjectComponent.h>
#include <Engine/File/ModelData.h>
#include <set>

namespace engine
{
	class Scene;
	namespace internal
	{
		class JoltInstance;
	}
}

namespace engine::physics
{
	class PhysicsManager;

	/**
	* @brief
	* Describes the result of a collision query.
	*/
	struct HitResult
	{
		/// True if anything has been hit.
		bool Hit = false;
		/// The penetration depth of the collision.
		float Depth = 0.0f;
		/// For ray/shape casts only. The distance at which something has been hit. From 0 (start of the ray) to 1 (end of the ray)
		float Distance = INFINITY;
		/// The component that has been hit. For the hit object, call `HitComponent->GetParent()`.
		ObjectComponent* HitComponent = nullptr;
		/// Impact point of the collision.
		Vector3 ImpactPoint;
		/// Surface normal of the collision.
		Vector3 Normal;

		static HitResult GetAverageHit(std::vector<HitResult> Hits);
	};

	/**
	* @brief
	* Physics layer enum. Defines collision rules for physics objects.
	* 
	* Each layer is one bit, so they can be combined. Example:
	* ```cpp
	* Body.CollisionLayers = Dynamic | Layer0;
	* ```
	* 
	* @ingroup Physics
	*/
	enum class Layer : uint16_t
	{
		/// No layer.
		None    = 0b00000000,
		/// Collide with everything else on this layer.
		Layer0  = 0b00000001,
		/// Collide with everything else on this layer.
		Layer1  = 0b00000010,
		/// Collide with everything else on this layer.
		Layer2  = 0b00000100,
		/// Collide with everything else on this layer.
		Layer3  = 0b00001000,
		/// Collide with everything else on this layer.
		Layer4  = 0b00010000,
		/// @todo Implement or replace with another "Layer" collision type.
		Trigger = 0b00100000,
		/// Will collide with everything.
		Static  = 0b01000000,
		/// Will only collide with static objects.
		Dynamic = 0b10000000
	};

	/**
	* @brief
	* Defines the motion type a PhysicsBody can have.
	* @ingroup Physics
	*/
	enum class MotionType
	{
		/// PhysicsBody cannot move.
		Static = 0,
		/// PhysicsBody is only movable using velocities only, does not respond to forces.
		Kinematic = 1,
		/// PhysicsBody is fully movable.
		Dynamic = 2,
	};

	inline Layer operator|(Layer a, Layer b)
	{
		return static_cast<Layer>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
	}

	inline Layer operator&(Layer a, Layer b)
	{
		return static_cast<Layer>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
	}


	/**
	* @brief
	* Describes a simulated body for the physics system.
	*
	* This class is virtual.
	* @ingroup Physics
	*/
	struct PhysicsBody
	{
		/// The type a physics body can have.
		enum class BodyType
		{
			Box,
			Sphere,
			Capsule,
			Mesh,
		};

		PhysicsManager* Manager = nullptr;

		BodyType Type = BodyType::Box;

		/**
		* @brief
		* Describes the movability for the body.
		*/
		MotionType ColliderMovability = MotionType::Static;

		/// The layers of the body.
		Layer CollisionLayers = Layer::None;
		bool IsCreated = false;
		bool IsActive = true;
		bool IsCollisionEnabled = true;
		/// The component the body belongs to.
		ObjectComponent* Parent = nullptr;
		PhysicsBody(BodyType NativeType, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent);

		/**
		* @brief
		* Gets the bodies position in the physics simulation.
		*/
		Vector3 GetPosition();
		/**
		* @brief
		* Gets the bodies rotation in the physics simulation
		*/
		Rotation3 GetRotation();

		void SetPosition(Vector3 NewPosition);
		void SetRotation(Rotation3 NewRotation);
		void SetPositionAndRotation(Vector3 NewPosition, Rotation3 NewRotation);

		void Activate();
		void Deactivate();

		void EnableCollision();
		void DisableCollision();


		/**
		* @brief
		* Scales the body by the given amount.
		*
		* Note: This scale is relative to the current scale.
		* An object that has been scaled by 0.5, then 2 will have it's initial scale.
		*/
		void Scale(Vector3 ScaleMultiplier);

		/**
		* @brief
		* Adds a physics force to the object.
		*
		* @param Direction
		* The direction of the force, with it's length being the force's strength.
		*
		* @param Point
		* The point where the physics force should be applied.
		*/
		void AddForce(Vector3 Direction, Vector3 Point = 0);

		void SetVelocity(Vector3 NewVelocity);
		void SetAngularVelocity(Vector3 NewVelocity);

		Vector3 GetVelocity();
		Vector3 GetAngularVelocity();

		std::vector<HitResult> CollisionTest(Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});
		std::vector<HitResult> ShapeCast(Transform StartTransform, Vector3 EndPos, Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});

		/// The *initial* Transform of the body.
		Transform BodyTransform;

		void* PhysicsSystemBody = nullptr;
		void* ShapeInfo = nullptr;
	protected:
	private:
	};

	/**
	* @brief
	* A PhysicsBody representing a sphere.
	*
	* @ingroup Physics
	*/
	struct SphereBody : public PhysicsBody
	{
		/**
		* @brief
		* Box constructor.
		*
		* @param Position
		* The position of the new sphere.
		* @param Rotation
		* The rotation of the new sphere.
		* @param Scale
		* The scale of the sphere.
		* @param ColliderMovability
		* The movability of the collider.
		* @param CollisionLayers
		* The Layers this collider should be active on.
		* @param Parent
		* The component this collider belongs to. Can be nullptr.
		*/

		SphereBody(Vector3 Position, Vector3 Rotation, float Scale, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent);
	};

	/**
	* @brief
	* A PhysicsBody representing a capsule.
	*
	* @ingroup Physics
	*/
	struct CapsuleBody : public PhysicsBody
	{
		/**
		* @brief
		* Capsule constructor.
		*
		* @param Position
		* The position of the new capsule.
		* @param Rotation
		* The rotation of the new capsule.
		* @param Scale
		* The scale of the new capsule. X is width, Y is height.
		* @param ColliderMovability
		* The movability of the collider.
		* @param CollisionLayers
		* The Layers this collider should be active on.
		* @param Parent
		* The component this collider belongs to. Can be nullptr.
		*/
		CapsuleBody(Vector3 Position, Vector3 Rotation, Vector2 Scale, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent);
	};

	/**
	* @brief
	* A PhysicsBody representing a box.
	*
	* @ingroup Physics
	*/
	struct BoxBody : public PhysicsBody
	{
		/**
		* @brief
		* Box constructor.
		*
		* @param Position
		* The position of the new box.
		* @param Rotation
		* The rotation of the new box.
		* @param Extents
		* The xyz extent of the new box.
		* @param ColliderMovability
		* The movability of the collider.
		* @param CollisionLayers
		* The Layers this collider should be active on.
		* @param Parent
		* The component this collider belongs to. Can be nullptr.
		*/
		BoxBody(Vector3 Position, Vector3 Rotation, Vector3 Extents, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent);

		Vector3 Extents;
	};

	/**
	* @brief
	* A PhysicsBody representing a polygon mesh.
	*
	* Note: This PhysicsBody only supports static movability. (See @ref MotionType)
	*
	* @ingroup Physics
	*/
	struct MeshBody : public PhysicsBody
	{
		/**
		* @brief
		* Mesh constructor.
		*
		* @param Mesh
		* The mesh data for the collider.
		* @param MeshTransform
		* Position, rotation and scale for the collider.
		* @param ColliderMovability
		* The movability of the collider.
		* @param CollisionLayers
		* The Layers this collider should be active on.
		* @param Parent
		* The component this collider belongs to. Can be nullptr.
		*/
		MeshBody(GraphicsModel* Mesh, Transform MeshTransform, MotionType ColliderMovability, Layer CollisionLayers, ObjectComponent* Parent);
		GraphicsModel* Model = nullptr;
	};


	class PhysicsManager
	{
	public:
		bool Active = true;
		PhysicsManager(Scene* From);

		void Init();
		void Update();

		/**
		* @brief
		* Casts a ray from the given start point to the end point.
		*
		* @param Start
		* Where the ray should start
		*
		* @param End
		* Where the ray should end.
		*
		* @param Layers
		* The layers on which collision should be tested.
		*
		* @param ObjectsToIgnore
		* A list of objects that should be ignored in the collision test.
		*
		* @return
		* A HitResult with HitResult.Hit = false if nothing was between the start and end points,
		* or a HitResult with HitResult.Hit = true and information about the first intersection of the ray if something is between the start and end points.
		*/
		HitResult RayCast(Vector3 Start, Vector3 End, Layer Layers, std::set<SceneObject*> ObjectsToIgnore = {});

		void AddBody(PhysicsBody* Body, bool StartActive, bool StartCollisionEnabled);
		void RemoveBody(PhysicsBody* Body);

		friend struct PhysicsBody;

	private:
		Scene* ParentScene = nullptr;
		internal::JoltInstance* PhysicsSystem = nullptr;
	};
}
#pragma once
#include "ObjectComponent.h"
#include "PhysicsComponent.h"

namespace engine
{
	class MoveComponent : public ObjectComponent
	{
	public:
		MoveComponent();
		void OnAttached() override;
		void Update() override;
		// Tries to move the object in the given direction.
		Vector3 TryMove(Vector3 Direction, Vector3 InitialDirection, Vector3 Pos, bool GravityPass, uint32 Depth = 0);
		/// Returns true if the object is touching the ground, false if not.
		bool GetIsOnGround() const;

		/// Gets the velocity the movement is at.
		Vector3 GetVelocity() const;
		void SetVelocity(Vector3 NewVelocity);

		/**
		* @brief
		* Adds an input to the movement. The movement will try to move in this direction.
		*/
		void AddMovementInput(Vector3 Direction);

		/// Sets the vertical velocity of the player to the "JumpHeight"
		void Jump();

		/// Jump height.
		float JumpHeight = 12;
		/// The maximum movement speed.
		float MaxSpeed = 5;
		/// The acceleration of the movement.
		float Acceleration = 50;
		/// The deceleration of the movement.
		float Deceleration = 40;
		/// The air acceleration multiplier. If the movement is in air, both acceleration and deceleration will be multiplied with this value.
		float AirAccelMultiplier = 0.5f;
		/// The gravity applied to the movement.
		float Gravity = 17;
		/// True if the movement is active.
		bool Active = true;
		bool LastMoveSuccessful = false;

		const uint32 MoveMaxDepth = 5;

		/// The size of the collider used by the movement. X is the capsule width, Y is the capsule height.
		Vector2 ColliderSize = Vector2(0.5f, 1.0f);

		Vector3 LastHitNormal;
		ObjectComponent* StoodOn = nullptr;

	private:
		PhysicsComponent* Collider = nullptr;

		Vector2 MovementVelocity;
		bool CanJump = true;
		float VerticalVelocity = 0;
		Vector3 InputDirection;
		Vector3 GroundNormal = Vector3(0, 1, 0);
		uint32 GroundedTimer = 0;
		bool HasBounced = false;
		bool Jumping = false;
		void* CollisionBodyPtr = nullptr;

	};
}
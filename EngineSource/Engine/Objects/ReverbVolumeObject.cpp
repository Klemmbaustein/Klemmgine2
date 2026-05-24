#include "ReverbVolumeObject.h"
#include <Engine/Scene.h>
#include <Engine/Engine.h>

void engine::ReverbVolumeObject::Begin()
{
	Volume = GetScene()->Sound->AddReverbVolume(this->ObjectTransform);

	if (!Engine::IsPlaying)
	{
		EditorCollider = new CollisionComponent();
		Attach(EditorCollider);
		EditorCollider->Load(GraphicsModel::UnitCube());
	}
}

void engine::ReverbVolumeObject::Update()
{
	if (EditorCollider)
	{
		EditorCollider->SetCollisionEnabled(GetScene()->Sound->ShowDebugBoundsInEditor);
	}

	if (this->Volume->AtTransform != this->ObjectTransform)
	{
		GetScene()->Sound->UpdateReverbVolumeTransform(this->Volume, this->ObjectTransform);
	}
}

void engine::ReverbVolumeObject::OnDestroyed()
{
	if (EditorCollider)
	{
		Detach(EditorCollider);
	}
	GetScene()->Sound->RemoveReverbVolume(Volume);
}

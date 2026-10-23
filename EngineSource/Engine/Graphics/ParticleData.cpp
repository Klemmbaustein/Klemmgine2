#include "ParticleData.h"
#include <Core/File/TextSerializer.h>
#include <Engine/File/Resource.h>
#include <sstream>

using namespace engine;

engine::graphics::ParticleData::ParticleData(AssetRef FromFile)
{
	std::stringstream FileStream;

	FileStream << resource::GetTextFile(FromFile.FilePath);

	SerializedValue Data = TextSerializer::FromStream(FileStream);
	DeSerialize(&Data);
}

engine::graphics::ParticleData::ParticleData()
{
}

SerializedValue engine::graphics::ParticleData::Serialize()
{
	return SerializedValue();
}

void engine::graphics::ParticleData::DeSerialize(SerializedValue* From)
{
	auto& Emitters = From->At("emitters");

	for (auto& i : Emitters.GetArray())
	{
		this->Emitters.emplace_back().Data.DeSerialize(&i);
	}
}

SerializedValue engine::graphics::ParticleEmitterData::Serialize()
{
	return SerializedValue();
}

void engine::graphics::ParticleEmitterData::DeSerialize(SerializedValue* From)
{
	Material = new graphics::Material(AssetRef::Convert(From->At("material").GetString()),
		ShaderAttributes{ .IsInstanced = true });
}

void engine::graphics::ParticleEmitter::RunStep(float Delta, Transform ViewTransform)
{
	Transforms.clear();

	Vector3 Min = INFINITY;
	Vector3 Max = -INFINITY;

	for (auto& i : Instances)
	{
		i.Velocity += Data.Acceleration;
		i.Position += i.Velocity * Delta;
		i.Lifetime -= Delta;

		Min = Min.Min(i.Position - i.Scale);
		Max = Max.Max(i.Position + i.Scale);

		Transforms.push_back(Transform(i.Position, Rotation3(0, 0, i.Rotation), i.Scale)
			.Combine(ViewTransform));
	}

	Bounds = BoundingBox::FromMinMax(Min, Max);

	NewParticleTimer -= Delta;

	if (NewParticleTimer <= 0)
	{
		Instances.push_back(ParticleInstance{
			.Position = Data.StartPosition,
			.Velocity = Data.Velocity,
			.Rotation = Data.Rotation,
			.Scale = Data.StartScale,
			.Lifetime = Data.Lifetime,
			});
		NewParticleTimer = 0.25f;
	}
}

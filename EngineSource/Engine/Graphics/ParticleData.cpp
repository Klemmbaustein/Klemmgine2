#include "ParticleData.h"
#include <Core/File/TextSerializer.h>
#include <Engine/File/Resource.h>
#include <sstream>
#include <random>
#include <Core/Log.h>

using namespace engine;

std::default_random_engine RandomGenerator;

engine::graphics::ParticleData::ParticleData(AssetRef FromFile)
{
	if (!FromFile.Exists())
	{
		return;
	}

	std::stringstream FileStream;

	FileStream << resource::GetTextFile(FromFile.FilePath);

	try
	{
		SerializedValue Data = TextSerializer::FromStream(FileStream);
		DeSerialize(&Data);
	}
	catch (SerializeException& e)
	{
		Log::Error(e.what());
	}
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

	VelocityRandom = From->At("velocityRandom").GetVector3();
	Velocity = From->At("velocity").GetVector3();
	StartScale = From->At("startScale").GetFloat();
	EndScale = From->At("endScale").GetFloat();
	Lifetime = From->At("lifetime").GetFloat();
}

void engine::graphics::ParticleEmitter::RunStep(float Delta, Transform ViewTransform)
{
	Transforms.clear();

	Vector3 Min = INFINITY;
	Vector3 Max = -INFINITY;

	GetInactiveInstances().clear();

	for (ParticleInstance i : GetInstances())
	{
		i.Velocity += Data.Acceleration * Delta;
		i.Position += i.Velocity * Delta;
		i.Scale = std::lerp(Data.EndScale, Data.StartScale, i.Lifetime / Data.Lifetime);
		i.Lifetime -= Delta;
		i.Rotation += i.RotationVelocity * Delta;

		Min = Min.Min(i.Position - i.Scale);
		Max = Max.Max(i.Position + i.Scale);

		Transform t = Transform(i.Position, Rotation3(), i.Scale)
			.Combine(ViewTransform);

		t.RotateAround(Vector3(0, 0, 1), i.Rotation);

		Transforms.push_back(t);
		if (i.Lifetime > 0)
		{
			GetInactiveInstances().push_back(i);
		}
	}

	Bounds = BoundingBox::FromMinMax(Min, Max);

	NewParticleTimer -= Delta;
	CurrentActiveInstances = !CurrentActiveInstances;

	auto RandomVec = [](Vector3 Range) {
		std::uniform_real_distribution<float> X{ -Range.X, Range.X };
		std::uniform_real_distribution<float> Y{ -Range.Y, Range.Y };
		std::uniform_real_distribution<float> Z{ -Range.Z, Range.Z };
		return Vector3(X(RandomGenerator), Y(RandomGenerator), Z(RandomGenerator));
	};

	if (NewParticleTimer <= 0)
	{
		GetInstances().push_back(ParticleInstance{
			.Position = Data.StartPosition + RandomVec(Data.StartRandom),
			.Velocity = Data.Velocity + RandomVec(Data.VelocityRandom),
			.Rotation = Data.Rotation,
			.RotationVelocity = Data.RotationVelocity,
			.Scale = Data.StartScale,
			.Lifetime = Data.Lifetime,
			});
		NewParticleTimer = 0.1f;
	}
}
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

engine::graphics::ParticleEmitterData::ParticleEmitterData()
{
	Material = graphics::Material::MakeDefault(ShaderAttributes{ .IsInstanced = true, });
}

engine::graphics::ParticleData::ParticleData()
{
}

SerializedValue engine::graphics::ParticleData::Serialize()
{
	SerializedValue Emitters = std::vector<SerializedValue>();

	for (auto& i : this->Emitters)
	{
		Emitters.Append(i.Data.Serialize());
	}

	return SerializedValue({
		SerializedData("emitters", Emitters),
		SerializedData("automaticBounds", AutomaticBoundingVolume),
		});
}

void engine::graphics::ParticleData::DeSerialize(SerializedValue* From)
{
	auto& Emitters = From->At("emitters");

	for (auto& i : Emitters.GetArray())
	{
		this->Emitters.emplace_back().Data.DeSerialize(&i);
	}

	AutomaticBoundingVolume = From->At("automaticBounds").GetBool();
}

SerializedValue engine::graphics::ParticleEmitterData::Serialize()
{
	return SerializedValue({
		SerializedData("material", MaterialFile.FilePath),
		SerializedData("position", StartPosition),
		SerializedData("positionRandom", StartRandom),
		SerializedData("velocityRandom", VelocityRandom),
		SerializedData("velocity", Velocity),
		SerializedData("acceleration", Acceleration),
		SerializedData("startScale", StartScale),
		SerializedData("endScale", EndScale),
		SerializedData("lifetime", Lifetime),
		SerializedData("spawnDelay", SpawnDelay),
		SerializedData("spawnCount", SpawnCount),
		SerializedData("rotation", Rotation),
		SerializedData("rotationVelocity", RotationVelocity),
		SerializedData("rotationVelocityRandom", RotationVelocityRandom),
		});
}

void engine::graphics::ParticleEmitterData::ReloadMaterial()
{
	if (Material)
	{
		delete Material;
		Material = nullptr;
	}
	Material = new graphics::Material(MaterialFile,
		ShaderAttributes{ .IsInstanced = true });
}

void engine::graphics::ParticleEmitterData::DeSerialize(SerializedValue* From)
{
	MaterialFile = AssetRef::Convert(From->At("material").GetString());
	ReloadMaterial();

	StartPosition = From->At("position").GetVector3();
	StartRandom = From->At("positionRandom").GetVector3();
	Velocity = From->At("velocity").GetVector3();
	VelocityRandom = From->At("velocityRandom").GetVector3();
	Acceleration = From->At("acceleration").GetVector3();
	StartScale = From->At("startScale").GetFloat();
	EndScale = From->At("endScale").GetFloat();
	Lifetime = From->At("lifetime").GetFloat();
	SpawnDelay = From->At("spawnDelay").GetFloat();
	SpawnCount = From->At("spawnCount").GetInt();
	Rotation = From->At("rotation").GetFloat();
	RotationVelocity = From->At("rotationVelocity").GetFloat();
	RotationVelocityRandom = From->At("rotationVelocityRandom").GetFloat();
}

bool engine::graphics::ParticleEmitter::IsPlaying() const
{
	return CurrentInstanceCount || Data.SpawnCount <= 0 || SpawnCount < Data.SpawnCount;
}

void engine::graphics::ParticleEmitter::RunStep(float Delta, Transform ViewTransform, bool AutomaticBoundingVolume)
{
	std::lock_guard g{ EmitterMutex };
	Transforms.clear();

	Vector3 Min = INFINITY;
	Vector3 Max = -INFINITY;

	GetInactiveInstances().clear();

	uint32 Count = 0;

	for (ParticleInstance i : GetInstances())
	{
		i.Velocity += Data.Acceleration * Delta;
		i.Position += i.Velocity * Delta;
		i.Scale = std::lerp(Data.EndScale, Data.StartScale, i.Lifetime / Data.Lifetime);
		i.Lifetime -= Delta;
		i.Rotation += i.RotationVelocity * Delta;

		if (AutomaticBoundingVolume)
		{
			Min = Min.Min(i.Position - i.Scale);
			Max = Max.Max(i.Position + i.Scale);
		}

		Transform t = Transform(i.Position, Rotation3(), i.Scale)
			.Combine(ViewTransform);

		t.RotateAround(Vector3(0, 0, 1), i.Rotation);

		Transforms.push_back(t);
		if (i.Lifetime > 0)
		{
			GetInactiveInstances().push_back(i);
		}
	}

	if (AutomaticBoundingVolume)
	{
		Bounds = BoundingBox::FromMinMax(Min, Max);
	}

	NewParticleTimer -= Delta;
	CurrentActiveInstances = !CurrentActiveInstances;

	auto RandomVec = [](Vector3 Range) {
		std::uniform_real_distribution<float> X{ -Range.X, Range.X };
		std::uniform_real_distribution<float> Y{ -Range.Y, Range.Y };
		std::uniform_real_distribution<float> Z{ -Range.Z, Range.Z };
		return Vector3(X(RandomGenerator), Y(RandomGenerator), Z(RandomGenerator));
	};

	auto CanSpawn = [this] {
		return;
	};


	while (NewParticleTimer <= 0 && (SpawnCount < Data.SpawnCount || Data.SpawnCount <= 0))
	{
		std::uniform_real_distribution<float> VelocityRng{ -Data.RotationVelocityRandom, Data.RotationVelocityRandom };

		GetInstances().push_back(ParticleInstance{
			.Position = Data.StartPosition + RandomVec(Data.StartRandom),
			.Velocity = Data.Velocity + RandomVec(Data.VelocityRandom),
			.Rotation = Data.Rotation,
			.RotationVelocity = Data.RotationVelocity + VelocityRng(RandomGenerator),
			.Scale = Data.StartScale,
			.Lifetime = Data.Lifetime,
			});
		NewParticleTimer = Data.SpawnDelay;
		SpawnCount++;
		if (Data.SpawnCount <= 0)
		{
			break;
		}
	}

	CurrentInstanceCount = int32(GetInstances().size());
}

void engine::graphics::ParticleEmitter::Reset()
{
	std::lock_guard g{ EmitterMutex };
	Transforms.clear();
	Instances1.clear();
	Instances2.clear();
	SpawnCount = 0;
	MainThreadInstanceCount = 0;
}

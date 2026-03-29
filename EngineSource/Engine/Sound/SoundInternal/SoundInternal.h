#include <AL/al.h>
#include <AL/alc.h>

namespace engine::sound
{
	struct SoundContext_Private
	{
		ALCcontext* Context;
	};

	struct SoundDevice_Private
	{
		ALCdevice* Device;
	};

	struct SoundBuffer
	{
		ALuint ALBuffer = 0;
		size_t References = 1;
	};
	struct SoundSource
	{
		ALuint ALSource = 0;
	};
}

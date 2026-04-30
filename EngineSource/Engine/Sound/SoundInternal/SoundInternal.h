#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

namespace engine::sound
{
	struct SoundContext_Private
	{
		ALCcontext* Context = nullptr;

		ALuint Effects;
	};

	struct SoundDevice_Private
	{
		ALCdevice* Device;

		LPALGENFILTERS alGenFilters = nullptr;
		LPALDELETEFILTERS alDeleteFilters = nullptr;
		LPALISFILTER alIsFilter = nullptr;
		LPALFILTERI alFilteri = nullptr;
		LPALFILTERIV alFilteriv = nullptr;
		LPALFILTERF alFilterf = nullptr;
		LPALFILTERFV alFilterfv = nullptr;
		LPALGETFILTERI alGetFilteri = nullptr;
		LPALGETFILTERIV alGetFilteriv = nullptr;
		LPALGETFILTERF alGetFilterf = nullptr;
		LPALGETFILTERFV alGetFilterfv = nullptr;

		LPALGENEFFECTS alGenEffects = nullptr;
		LPALDELETEEFFECTS alDeleteEffects = nullptr;
		LPALISEFFECT alIsEffect = nullptr;
		LPALEFFECTI alEffecti = nullptr;
		LPALEFFECTIV alEffectiv = nullptr;
		LPALEFFECTF alEffectf = nullptr;
		LPALEFFECTFV alEffectfv = nullptr;
		LPALGETEFFECTI alGetEffecti = nullptr;
		LPALGETEFFECTIV alGetEffectiv = nullptr;
		LPALGETEFFECTF alGetEffectf = nullptr;
		LPALGETEFFECTFV alGetEffectfv = nullptr;

		LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
		LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
		LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
		LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
		LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
		LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
		LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
		LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
		LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
		LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
		LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;
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

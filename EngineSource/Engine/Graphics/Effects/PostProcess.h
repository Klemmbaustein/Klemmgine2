#pragma once
#include "PostProcessEffect.h"
#include <Core/Types.h>
#include <array>
#include <Engine/Graphics/Framebuffer.h>

namespace engine::graphics
{
	class PostProcess
	{
	public:
		PostProcess();
		~PostProcess();

		void Init(uint32 Width, uint32 Height);

		void AddEffect(PostProcessEffect* NewEffect);
		void RemoveEffect(PostProcessEffect* TargetEffect);

		uint32 Draw(Framebuffer* Buffer, Camera* Cam);
		void OnBufferResized(uint32 Width, uint32 Height);

		// Returns post process framebuffer and texture objects.
		std::pair<uint32, uint32> NextBuffer();

		template<typename T>
		T* GetEffect()
		{
			for (PostProcessEffect* i : ActiveEffects)
			{
				if (typeid(*i) == typeid(T))
				{
					return static_cast<T*>(i);
				}
			}
			return nullptr;
		}

	private:

		void SortEffectsByPriority();
		void GenerateBuffers();

		uint32 Width = 0, Height = 0;

		std::vector<PostProcessEffect*> ActiveEffects;
		std::array<uint32, 2> PostProcessBuffers;
		std::array<uint32, 2> PostProcessTextures;
		bool IsFirstBuffer = true;
	};
}
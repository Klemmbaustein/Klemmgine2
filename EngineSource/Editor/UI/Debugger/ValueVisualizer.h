#pragma once
#include <ds/debug/debugState.hpp>
#include <Core/Types.h>

namespace engine::editor
{
	class ValueVisualizer
	{
	public:

		ValueVisualizer() = default;
		virtual ~ValueVisualizer() = default;

		virtual bool SupportsType(const ds::DebugVariable& Variable) = 0;
		virtual string GetPreviewText(const ds::DebugVariable& Variable) = 0;
		virtual std::vector<ds::DebugVariable> GetMembers(const ds::DebugVariable& Variable) = 0;

		int32 Priority = 0;
	};
}
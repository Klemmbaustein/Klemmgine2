#pragma once
#include <Editor/UI/Debugger/ClassVisualizer.h>

namespace engine::editor
{
	class ArrayVisualizer : public ClassVisualizer
	{
	public:

		ArrayVisualizer();

		bool SupportsType(const ds::DebugVariable& Variable) override;
		std::vector<ds::DebugVariable> GetMembers(const ds::DebugVariable& Variable) override;

	private:
		std::vector<string> ArrayIndexStrings;
	};
}
#pragma once
#include <Editor/UI/Debugger/ValueVisualizer.h>

namespace engine::editor
{
	class ClassVisualizer : public ValueVisualizer
	{
	public:

		ClassVisualizer();

		// Inherited via ValueVisualizer
		bool SupportsType(const ds::DebugVariable& Variable) override;

		string GetPreviewText(const ds::DebugVariable& Variable) override;

		std::vector<ds::DebugVariable> GetMembers(const ds::DebugVariable& Variable) override;
	};

	class StringVisualizer : public ClassVisualizer
	{
	public:
		StringVisualizer();

		bool SupportsType(const ds::DebugVariable& Variable) override;
		string GetPreviewText(const ds::DebugVariable& Variable) override;
	};
}
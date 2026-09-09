#pragma once
#include <Editor/UI/Debugger/ValueVisualizer.h>

namespace engine::editor
{
	class PrimitiveVisualizer : public ValueVisualizer
	{
	public:
		ds::TypeId PrimitiveType = 0;

		// Inherited via ValueVisualizer
		bool SupportsType(const ds::DebugVariable& Variable) override;
		std::vector<ds::DebugVariable> GetMembers(const ds::DebugVariable& Variable) override;
	};

	class IntVisualizer : public PrimitiveVisualizer
	{
	public:
		IntVisualizer();

		// Inherited via PrimitiveVisualizer
		string GetPreviewText(const ds::DebugVariable& Variable) override;
	};

	class FloatVisualizer : public PrimitiveVisualizer
	{
	public:
		FloatVisualizer();

		// Inherited via PrimitiveVisualizer
		string GetPreviewText(const ds::DebugVariable& Variable) override;
	};

	class BoolVisualizer : public PrimitiveVisualizer
	{
	public:
		BoolVisualizer();

		// Inherited via PrimitiveVisualizer
		string GetPreviewText(const ds::DebugVariable& Variable) override;
	};
}
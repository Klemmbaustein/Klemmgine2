#include "PrimitiveVisualizer.h"
#include <ds/parser/types/type.hpp>

using namespace engine;
using namespace ds;

bool engine::editor::PrimitiveVisualizer::SupportsType(const ds::DebugVariable& Variable)
{
	return Variable.isPrimitive && Variable.type == PrimitiveType;
}

std::vector<ds::DebugVariable> engine::editor::PrimitiveVisualizer::GetMembers(const ds::DebugVariable& Variable)
{
	return {};
}

engine::editor::IntVisualizer::IntVisualizer()
{
	this->PrimitiveType = IntType::INT_ID;
}

string engine::editor::IntVisualizer::GetPreviewText(const ds::DebugVariable& Variable)
{
	return std::to_string(*reinterpret_cast<Int*>(Variable.pointer));
}

engine::editor::FloatVisualizer::FloatVisualizer()
{
	this->PrimitiveType = FloatType::FLOAT_ID;
}

string engine::editor::FloatVisualizer::GetPreviewText(const ds::DebugVariable& Variable)
{
	return str::FloatToString(*reinterpret_cast<Float*>(Variable.pointer));
}

engine::editor::BoolVisualizer::BoolVisualizer()
{
	this->PrimitiveType = BoolType::BOOL_ID;
}

string engine::editor::BoolVisualizer::GetPreviewText(const ds::DebugVariable& Variable)
{
	return *reinterpret_cast<Bool*>(Variable.pointer) ? "true" : "false";
}

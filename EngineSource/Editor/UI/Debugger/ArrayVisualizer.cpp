#include "ArrayVisualizer.h"
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/parser/types/arrayType.hpp>
#include <ds/class.hpp>
#include <ds/modules/system.hpp>

using namespace engine::script;
using namespace engine;
using namespace ds;

engine::editor::ArrayVisualizer::ArrayVisualizer()
{
	Priority = 100;
}

bool engine::editor::ArrayVisualizer::SupportsType(const ds::DebugVariable& Variable)
{
	auto& Reflect = ScriptSubsystem::Instance->ScriptInstructions->reflect;

	auto Found = Reflect.types.find(Variable.type);

	if (Found != Reflect.types.end())
	{
		return Found->second.implementsGeneric == ds::ArrayType::ARRAY_BASE_TYPE;
	}
	return false;
}

std::vector<ds::DebugVariable> engine::editor::ArrayVisualizer::GetMembers(const ds::DebugVariable& Variable)
{
	auto BaseMembers = ClassVisualizer::GetMembers(Variable);
	auto& Reflect = ScriptSubsystem::Instance->ScriptInstructions->reflect;

	ClassRef<modules::system::ArrayData> Array = reinterpret_cast<ds::RuntimeClass*>(Variable.pointer);

	auto BaseTypeId = Variable.type - ds::ArrayType::ARRAY_BASE_TYPE;
	auto FoundBaseType = Reflect.types.find(BaseTypeId);

	size_t FoundTypeSize = 0;
	bool FoundIsPrimitive = true;

	if (FoundBaseType != Reflect.types.end())
	{
		FoundIsPrimitive = false;
		FoundTypeSize = sizeof(RuntimeClass*);
	}
	else if (BaseTypeId == IntType::INT_ID)
	{
		FoundTypeSize = sizeof(ds::Int);
	}
	else if (BaseTypeId == FloatType::FLOAT_ID)
	{
		FoundTypeSize = sizeof(ds::Float);
	}
	else if (BaseTypeId == BoolType::BOOL_ID)
	{
		FoundTypeSize = sizeof(ds::Bool);
	}

	if (FoundTypeSize == 0)
	{
		return BaseMembers;
	}

	ArrayIndexStrings.resize(std::max<size_t>(ArrayIndexStrings.size(), Array->length));

	for (size_t i = 0; i < Array->length; i++)
	{
		auto& str = ArrayIndexStrings[i];
		if (str.empty())
		{
			str.assign(str::Format("[%i]", i));
		}

		if (FoundIsPrimitive)
		{
			BaseMembers.push_back(DebugVariable{
				.pointer = reinterpret_cast<uByte*>(Array->data) + FoundTypeSize * i,
				.name = str.c_str(),
				.type = BaseTypeId,
				.isPrimitive = FoundIsPrimitive,
				});
		}
		else
		{
			BaseMembers.push_back(DebugVariable{
				.pointer = *reinterpret_cast<void**>(reinterpret_cast<uByte*>(Array->data) + FoundTypeSize * i),
				.name = str.c_str(),
				.type = BaseTypeId,
				.isPrimitive = FoundIsPrimitive,
				});
		}
	}
	return BaseMembers;
}

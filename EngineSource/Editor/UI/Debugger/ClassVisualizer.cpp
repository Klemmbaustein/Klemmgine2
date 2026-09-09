#include "ClassVisualizer.h"
#include <Engine/Script/ScriptSubsystem.h>
#include <ds/parser/types/stringType.hpp>
#include <ds/runtimeString.hpp>

using namespace engine;
using namespace engine::script;

engine::editor::ClassVisualizer::ClassVisualizer()
{
	Priority = -100;
}

bool engine::editor::ClassVisualizer::SupportsType(const ds::DebugVariable& Variable)
{
	auto& Reflect = ScriptSubsystem::Instance->ScriptInstructions->reflect;

	auto foundType = Reflect.types.find(Variable.type);
	if (foundType == Reflect.types.end())
	{
		if (Variable.isPrimitive || !Variable.pointer)
		{
			return false;
		}
		ds::RuntimeClass* ClassValue = reinterpret_cast<ds::RuntimeClass*>(Variable.pointer);
		foundType = Reflect.types.find(ClassValue->type);

		return foundType != Reflect.types.end();
	}

	return true;

	// return /*!Variable.isPrimitive*/ true;
}

string engine::editor::ClassVisualizer::GetPreviewText(const ds::DebugVariable& Variable)
{
	if (!Variable.pointer)
	{
		return "null";
	}
	auto& Debug = ScriptSubsystem::Instance->ScriptInstructions->debug;
	auto& Reflect = ScriptSubsystem::Instance->ScriptInstructions->reflect;

	if (Variable.isPrimitive)
	{
		return Reflect.types[Variable.type].name;
	}

	ds::RuntimeClass* ClassValue = reinterpret_cast<ds::RuntimeClass*>(Variable.pointer);

	auto found = Reflect.types.find(ClassValue->type);

	if (!ClassValue->type || found == Reflect.types.end())
	{
		return str::Format("%s <extern> <0x%llx>", Reflect.types[Variable.type].name.c_str(), size_t(ClassValue));
	}

	return str::Format("%s <0x%llx>", found->second.name.c_str(), size_t(ClassValue));
}

std::vector<ds::DebugVariable> engine::editor::ClassVisualizer::GetMembers(const ds::DebugVariable& Variable)
{
	if (!Variable.pointer)
	{
		return {};
	}
	auto& Debug = ScriptSubsystem::Instance->ScriptInstructions->debug;
	auto& Reflect = ScriptSubsystem::Instance->ScriptInstructions->reflect;

	ds::RuntimeClass* ClassValue = reinterpret_cast<ds::RuntimeClass*>(Variable.pointer);

	auto found = Reflect.types.find(ClassValue->type);

	ds::TypeId Type = Variable.type;

	if (found != Reflect.types.end())
	{
		Type = ClassValue->type;
	}

	auto foundType = Debug.classInfo.find(Type);

	if (foundType == Debug.classInfo.end())
	{
		return {};
	}

	auto& DebugInfo = Debug.classInfo[Type];

	std::vector<ds::DebugVariable> Members;

	for (auto& i : DebugInfo.members)
	{
		if (i.isPointerMember)
		{
			continue;
		}

		if (Variable.isPrimitive)
		{
			Members.push_back(ds::DebugVariable{
				.pointer = reinterpret_cast<uByte*>(ClassValue) + i.offset,
				.name = i.name.c_str(),
				.type = i.type,
				.isPrimitive = i.isPrimitive,
				});
		}
		else
		{
			Members.push_back(ds::DebugVariable{
				.pointer = i.isPrimitive ? (ClassValue->getBody() + i.offset) : *reinterpret_cast<void**>(ClassValue->getBody() + i.offset),
				.name = i.name.c_str(),
				.type = i.type,
				.isPrimitive = i.isPrimitive,
				});

		}
	}

	return Members;
}

engine::editor::StringVisualizer::StringVisualizer()
{
	this->Priority = 100;
}

bool engine::editor::StringVisualizer::SupportsType(const ds::DebugVariable& Variable)
{
	return Variable.type == ds::StringType::STRING_ID;
}

string engine::editor::StringVisualizer::GetPreviewText(const ds::DebugVariable& Variable)
{
	if (!Variable.pointer)
	{
		return "null";
	}
	ds::RuntimeStrRef ClassValue = reinterpret_cast<ds::RuntimeClass*>(Variable.pointer);

	return str::Format("\"%s\"", ClassValue.ptr());
}

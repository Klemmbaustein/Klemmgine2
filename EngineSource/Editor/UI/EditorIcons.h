#ifdef EDITOR
#pragma once
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <map>

namespace engine::editor
{

	class EditorIcons
	{
	public:
		void AddObjectIcon(string IconPath, ObjectTypeID ObjType);
		string GetObjectIcon(ObjectTypeID ObjType);

	private:
		std::map<ObjectTypeID, string> Icons;
		static constexpr const char* DEFAULT_ICON = "Engine/Editor/Assets/Object.png";
	};
}
#endif
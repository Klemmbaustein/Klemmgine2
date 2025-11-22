#include "EditorIcons.h"

using namespace engine;
using namespace engine::editor;

void EditorIcons::AddObjectIcon(string IconPath, ObjectTypeID ObjType)
{
	Icons[ObjType] = IconPath;
}

string EditorIcons::GetObjectIcon(ObjectTypeID ObjType)
{
	auto FoundIcon = Icons.find(ObjType);

	if (FoundIcon == Icons.end())
	{
		return DEFAULT_ICON;
	}

	return FoundIcon->second;
}

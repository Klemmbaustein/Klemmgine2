#ifdef EDITOR
#include "Editor.h"
#include <Engine/Subsystem/EditorSubsystem.h>
using namespace engine;
using namespace engine::subsystem;

const bool editor::IsActive()
{
	return EditorSubsystem::Active;
}
#endif
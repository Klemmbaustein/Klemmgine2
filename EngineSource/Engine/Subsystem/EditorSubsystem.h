#ifdef EDITOR
#pragma once
#include "Subsystem.h"

namespace engine::subsystem
{
	class EditorSubsystem : public Subsystem
	{
	public:
		EditorSubsystem();

		virtual void Update() override;
	};
}
#endif
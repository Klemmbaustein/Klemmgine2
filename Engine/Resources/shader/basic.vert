//? #version 330
#using "engine.common" //! #include "engine.common.frag"

void main()
{
	SetScreenPosition(WorldPosToScreenPos(a_position));
}

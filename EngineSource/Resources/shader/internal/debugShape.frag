#using "engine.base" //! #include "../engine.base.frag"
#unlit //!

#param //!
uniform vec3 u_color = vec3(1, 0, 0);

vec3 fragment()
{
    opacity = 0.25;
    return u_color;
}
//! #version 330

layout(location = 0) out vec4 f_color;
in vec2 v_texcoords;
uniform sampler2D u_texture;
uniform vec2 u_res;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    f_color.rgb = texture(u_texture, v_texcoords).rgb * weight[0]; // current fragment's contribution

    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            f_color.xyz += texture(u_texture, v_texcoords + vec2(u_res.x * i, 0.0)).rgb * weight[i];
            f_color.xyz += texture(u_texture, v_texcoords - vec2(u_res.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            f_color.xyz += texture(u_texture, v_texcoords + vec2(0.0, u_res.y * i)).rgb * weight[i];
            f_color.xyz += texture(u_texture, v_texcoords - vec2(0.0, u_res.y * i)).rgb * weight[i];
        }
    }
	f_color.w = 1;
}
//? #version 430

#if !ENGINE_GL_430
#extension GL_ARB_uniform_buffer_object : enable
#extension GL_ARB_gpu_shader5 : enable
#endif

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;
in vec2 v_texCoord[];
out vec2 g_texCoord;

layout (std140) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[16];
};

uniform int u_shadowCascadeCount = 3;

void main()
{
	for (int i = 0; i < u_shadowCascadeCount; ++i)
	{
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		g_texCoord = v_texCoord[i];
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}
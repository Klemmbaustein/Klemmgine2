// Common shading functions

//? #version 430
#module "engine.common" //!
#using "engine.base" //! #include "engine.base.frag"

#if !ENGINE_GL_430
#extension GL_ARB_uniform_buffer_object : enable
#extension GL_ARB_arrays_of_arrays : enable
#endif

#export //!
uniform vec3 u_lightDirection = vec3(0, 1, 0);

struct Light
{
	bool isActive;
	vec3 position;
	vec3 rangeFalloffIntensity;
	vec3 color;
};

uniform Light u_lights[8];

#export //!
uniform vec3 u_sunColor = vec3(1);
#export //!
uniform float u_sunIntensity = 1.0;
#export //!
uniform vec3 u_skyColor = vec3(1);
#export //!
uniform vec3 u_groundColor = vec3(1);
#export //!
uniform float u_ambientStrength = 0.2;

uniform bool u_drawShadows = true;

layout (std140) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[16];
};

uniform float cascadePlaneDistances[16];
uniform int u_shadowCascadeCount = 4;
uniform sampler2DArray u_shadowMaps;
uniform float u_shadowBiasModifier = 0;

uniform vec3 u_sceneFogColor = vec3(0.0);
uniform float u_sceneFogRange = 0.0;
uniform float u_sceneFogStart = 0.0;

#define PCF_SIZE 4
#define PCF_HALF_SIZE 2

float shadowValues[PCF_SIZE][PCF_SIZE];

vec3 getAmbient(vec3 normal)
{
	return mix(u_groundColor, u_skyColor, normal.y / 2.0 + 0.5);
}

float rand(vec2 n)
{
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float SampleFromShadowMap(vec2 distances)
{
	float ShadowVal = 0;
	for (int x = 0; x < PCF_SIZE - 1; ++x)
	{
		for (int y = 0; y < PCF_SIZE - 1; ++y)
		{
			ShadowVal += mix(mix(shadowValues[x][y], shadowValues[x + 1][y], distances.x),
				mix(shadowValues[x][y + 1], shadowValues[x + 1][y +  1], distances.x), distances.y);
		}
	}
	return 1 - clamp(ShadowVal / ((PCF_SIZE - 1) * (PCF_SIZE - 1)), 0.0, 1.0);
}

float lightStrength(vec3 at, float range, float falloff, float intensity)
{
#define Kc 1.0
#define Kl (falloff / 2.0)
#define Kq (falloff * 1.5)

	vec3 diff = v_position - at;

	float d = length(diff);

	vec3 dir = diff / vec3(d);

	float cutoutMultiplier = clamp((range - d), 0, 1);

	return 1.0 / (Kc + Kl * d + Kq * d * d) * intensity * cutoutMultiplier * max(dot(dir, -v_normal), 0.0);
}

float getShadowStrength()
{
	if (!u_drawShadows)
	{
		return 1.0;
	}

	// select cascade layer
	float depthValue = abs(v_screenPosition.z);

	int layer = u_shadowCascadeCount;
	float difference = 10;
	float differenceScale = 0;
	for (int i = 0; i < u_shadowCascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			differenceScale = cascadePlaneDistances[i] * 0.9;
			difference = cascadePlaneDistances[i] - depthValue;
			layer = i;
			break;
		}
	}

	if (difference < differenceScale && mod(rand(v_screenPosition.xy), differenceScale) > difference)
	{
		layer++;
	}

	if (layer >= u_shadowCascadeCount)
	{
		return 1.0;
	}

	vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(v_position, 1.0);

	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if (currentDepth > 1.0)
	{
		return 1.0;
	}

	float bias = (1 - (abs(dot(v_normal, u_lightDirection)))) / 75 + 0.01;
	bias *= max((abs(u_shadowBiasModifier * 1.25)), 0.5) / 10.0;
	if (u_shadowBiasModifier < -0.95)
		bias *= 1.5;
	bias *= 0.12;
	bias *= max(4096.0 / (textureSize(u_shadowMaps, 0).x * 1.0), 1.0);

	vec2 texelSize = 1.0 / textureSize(u_shadowMaps, 0).xy;
	bool allValuesLight = true;
	bool allValuesDark = true;
	for (int x = 0; x < PCF_SIZE; ++x)
	{
		for (int y = 0; y < PCF_SIZE; ++y)
		{
			float pcfDepth = texture(u_shadowMaps, vec3(projCoords.xy
				+ ivec2(x - PCF_HALF_SIZE, y - PCF_HALF_SIZE) * texelSize.xy, layer)).r;

			bool isLight = currentDepth > pcfDepth + bias;

			shadowValues[x][y] = isLight ? 1.0 : 0.0;
			if (isLight)
				allValuesLight = false;
			else
				allValuesDark = false;
		}
	}

	// No need to do smoothing if all values are the same.
	if (allValuesLight)
		return 1.0;
	if (allValuesDark)
		return 0.0;

	vec2 distances = vec2(mod(projCoords.x, texelSize.x),
		mod(projCoords.y, texelSize.y)) * vec2(textureSize(u_shadowMaps, 0));
	return SampleFromShadowMap(distances);
}

#export //!
float getLightStrength()
{
	return getShadowStrength() * max(dot(v_normal, u_lightDirection), 0.0);
}

#export //!
vec3 applyFog(vec3 color)
{
	return u_sceneFogRange > 0
		? mix(color, u_sceneFogColor, pow(clamp((length(v_screenPosition) - u_sceneFogStart) / u_sceneFogRange, 0.0, 1.0), 1.25))
		: color;
}

#export //!
vec3 applyLightingSpecular(vec3 color, float specularStength, float specularSize)
{
	float shadows = 1;
	if (u_sunIntensity > 0)
	{
		shadows = getLightStrength();
	}

	vec3 ambient = color * u_ambientStrength * getAmbient(v_normal);

	vec3 viewDir = normalize(u_cameraPos - v_position);
	vec3 reflectDir = reflect(-u_lightDirection, v_normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularSize);
	vec3 spec_color = spec * specularStength * u_sunColor * u_sunIntensity;

	vec3 lightValue = vec3(0);

	for (int i = 0; i < 8; i++)
	{
		if (!u_lights[i].isActive)
		{
			continue;
		}

		lightValue += vec3(lightStrength(u_lights[i].position, u_lights[i].rangeFalloffIntensity.x,
			u_lights[i].rangeFalloffIntensity.y, u_lights[i].rangeFalloffIntensity.z)) * u_lights[i].color;
	}

	return applyFog((color * u_sunColor * u_sunIntensity + spec_color) * shadows + ambient + lightValue * color);
}

#export //!
vec3 applyLighting(vec3 color)
{
	return applyLightingSpecular(color, 0.0, 1.0);
}

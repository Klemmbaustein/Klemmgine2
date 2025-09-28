#include "Environment.h"
#include "ShaderObject.h"

void engine::graphics::Environment::ApplyTo(ShaderObject* TargetShader) const
{
	TargetShader->SetVec3(TargetShader->GetUniformLocation("u_sunColor"), this->SunColor);
	TargetShader->SetVec3(TargetShader->GetUniformLocation("u_skyColor"), this->SkyColor);
	TargetShader->SetVec3(TargetShader->GetUniformLocation("u_groundColor"), this->GroundColor);
	TargetShader->SetFloat(TargetShader->GetUniformLocation("u_ambientStrength"), this->AmbientIntensity);
}

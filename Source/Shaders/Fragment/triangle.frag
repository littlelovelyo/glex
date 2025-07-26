#version 450 core

layout(location=0) in vec3 vColor;
layout(location=1) in vec3 vPos;
layout(location=2) in vec3 vCamPos;
layout(location=0) out vec4 fColor;

layout(set=1, binding=0) uniform samplerCube Skybox;

void main()
{
	vec3 dir = reflect(vPos - vCamPos, vec3(0.0, 1.0, 0.0));
	vec3 skyColor = texture(Skybox, dir).rgb;
	fColor = vec4(mix(vColor, skyColor, 0.5), 1.0);
}
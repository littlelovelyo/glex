#version 450 core

layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aColor;

layout(location=0) out vec3 vColor;
layout(location=1) out vec3 vPos;
layout(location=2) out vec3 vCamPos;

layout(set=0, binding=0) uniform GlobalData
{
	mat4 viewMat;
	mat4 projMat;
	vec2 screenSize;
} globalData;

layout(push_constant) uniform ObjectData
{
	mat4 modelMat;
} objectData;

void main()
{
	vec4 pos = objectData.modelMat * vec4(aPos.x, -1.2, -aPos.y, 1.0);
	gl_Position = globalData.projMat * globalData.viewMat * pos;
	vColor = aColor;
	vPos = pos.xyz;
	mat3 s = mat3(globalData.viewMat);
	vCamPos = -inverse(s) * globalData.viewMat[3].xyz;
}
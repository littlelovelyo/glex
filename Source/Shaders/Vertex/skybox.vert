#version 450 core

layout(location=0) in vec3 aPos;

layout(location=0) out vec3 vTexCoords;

layout(set=0, binding=0) uniform GlobalData
{
	mat4 viewMat;
	mat4 projMat;
} globalData;

void main()
{
	gl_Position = globalData.projMat * mat4(mat3(globalData.viewMat)) * vec4(aPos, 1.0);
	vTexCoords = aPos;
}
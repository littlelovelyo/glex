#version 450 core

layout(location=0) in vec2 aPos;
layout(location=1) in uint aTexID;
layout(location=2) in vec2 aUV;
layout(location=3) in vec4 aColor;

layout(location=0) flat out uint vTexID;
layout(location=1) out vec2 vUV;
layout(location=2) out vec4 vColor;

layout(set=0, binding=0) uniform GlobalData
{
	mat4 viewMat;
	mat4 projMat;
	vec2 screenSize;
} globalData;

void main()
{
	gl_Position = vec4(aPos / globalData.screenSize * 2.0 - 1.0, 0.0, 1.0);
	vTexID = aTexID;
	vUV = aUV;
	vColor = aColor;
}
#version 450 core

layout(location=0) flat in uint vTexID;
layout(location=1) in vec2 vUV;
layout(location=2) in vec4 vColor;

layout(set=2, binding=0) uniform sampler2D Textures[64];

layout(location=0) out vec4 fColor;

void main()
{
	if (vTexID == 0xffffffff)
		fColor = vColor;
	else
		fColor = vColor * texture(Textures[vTexID], vUV);
}
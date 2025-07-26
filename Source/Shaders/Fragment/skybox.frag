#version 450 core

layout(location=0) in vec3 vTexCoords;

layout(set=1, binding=0) uniform samplerCube Skybox;

layout(location=0) out vec4 fColor;

void main()
{
	fColor = texture(Skybox, vTexCoords);
}
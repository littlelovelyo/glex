#version 450 core

layout(set=1, binding=0) uniform Material
{
	vec3 diffuse;
	float specular;
	float shininess;
} material;

layout(location=0) out vec4 fColor;

void main()
{
	fColor = vec4(material.diffuse, 1.0);
}